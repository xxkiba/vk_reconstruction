#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace VKFW::renderer {

	void Model::loadModel(const std::string& path, const VKFW::Ref<VKFW::vulkancore::Device>& device) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::string warn;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
			throw std::runtime_error("Failed to load model: " + err);
		}

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				StaticMeshVertexData vertexData = {};
				// Vertex position
				mPositions.push_back(attrib.vertices[3 * index.vertex_index + 0]);
				mPositions.push_back(attrib.vertices[3 * index.vertex_index + 1]);
				mPositions.push_back(attrib.vertices[3 * index.vertex_index + 2]);

				vertexData.mPosition = glm::vec3(
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]);

				// Vertex color (using a default color for simplicity)
				mColors.push_back(1.0f);
				mColors.push_back(1.0f);
				mColors.push_back(1.0f);

				vertexData.mColor = glm::vec3(1.0f, 1.0f, 1.0f); // Default color
				// UV coordinates
				if (index.texcoord_index >= 0) {
					mUVs.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
					mUVs.push_back(1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);
					vertexData.mUV = glm::vec2(
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);
				}
				else {
					mUVs.push_back(0.0f);
					mUVs.push_back(0.0f);
					vertexData.mUV = glm::vec2(0.0f, 0.0f); // Default UV
				}
				// Index
				mIndexDatas.push_back(static_cast<unsigned int>(mIndexDatas.size()));
				vertexData.mNormal = glm::vec3(0.0f, 0.0f, 0.0f); // Default normal
				vertexData.mTangent = glm::vec3(0.0f, 0.0f, 0.0f); // Default tangent

				mVertexDatas.push_back(vertexData);

			}
		}

		//mPositionBuffer = VKFW::vulkancore::DataBuffer::createVertexBuffer(device, mPositions.size() * sizeof(float), mPositions.data());

		//mColorBuffer = VKFW::vulkancore::DataBuffer::createVertexBuffer(device, mColors.size() * sizeof(float), mColors.data());

		//mUVBuffer = VKFW::vulkancore::DataBuffer::createVertexBuffer(device, mUVs.size() * sizeof(float), mUVs.data());

		mVertexDataBuffer = VKFW::vulkancore::DataBuffer::createVertexBuffer(device, mVertexDatas.size() * sizeof(StaticMeshVertexData), mVertexDatas.data());

		mIndexBuffer = VKFW::vulkancore::DataBuffer::createIndexBuffer(device, mIndexDatas.size() * sizeof(unsigned int), mIndexDatas.data());

		setVertexInputBindingDescriptions();
		setAttributeDescription();

	}

	void Model::loadBattleFireModel(const std::string& path, const VKFW::Ref<VKFW::vulkancore::Device>& device) {
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);

		// 1. Read vertex count
		int vertexCount = 0;
		file.read(reinterpret_cast<char*>(&vertexCount), sizeof(int));
		if (!file) throw std::runtime_error("Failed to read vertex count");

		// 2. Read vertex data
		mBattleFireVertexDatas.resize(vertexCount);
		file.read(reinterpret_cast<char*>(mBattleFireVertexDatas.data()), sizeof(BattleFireMeshVertexData) * vertexCount);
		if (!file) throw std::runtime_error("Failed to read vertex data");

		//// 3. (Optional) You can also fill mPositions, mColors, mUVs if you want consistency with your OBJ loader
		//mPositions.clear();
		//mColors.clear();
		//mUVs.clear();
		//for (const auto& v : mBattleFireVertexDatas) {
		//	mPositions.push_back(v.mPosition.x);
		//	mPositions.push_back(v.mPosition.y);
		//	mPositions.push_back(v.mPosition.z);
		//	mColors.push_back(v.mColor.r);
		//	mColors.push_back(v.mColor.g);
		//	mColors.push_back(v.mColor.b);
		//	mUVs.push_back(v.mUV.x);
		//	mUVs.push_back(v.mUV.y);
		//}

		// 4. Create vertex buffer
		mVertexDataBuffer = VKFW::vulkancore::DataBuffer::createVertexBuffer(device, mBattleFireVertexDatas.size() * sizeof(BattleFireMeshVertexData), mBattleFireVertexDatas.data());

		// 5. Read submeshes until end of file
		mSubMeshes.clear();
		mIndexDatas.clear();
		mIndexBuffer.reset();

		while (file.peek() != EOF) {
			int nameLen = 0;
			file.read(reinterpret_cast<char*>(&nameLen), sizeof(int));
			if (!file || file.eof()) break;

			std::string name(nameLen, '\0');
			file.read(&name[0], nameLen);
			if (!file) break;

			int indexCount = 0;
			file.read(reinterpret_cast<char*>(&indexCount), sizeof(int));
			if (!file) break;

			std::vector<uint32_t> indices(indexCount);
			file.read(reinterpret_cast<char*>(indices.data()), sizeof(uint32_t) * indexCount);
			if (!file) break;

			// For global index buffer if needed (optional, for non-submesh drawing)
			mIndexDatas.insert(mIndexDatas.end(), indices.begin(), indices.end());

			// Create submesh
			SubMesh* submesh = new SubMesh;
			submesh->mIndexCount = indexCount;
			submesh->mSubMeshIndices = std::move(indices);

			// Create a Vulkan buffer for the submesh indices
			submesh->mSubMeshIndexBuffer = VKFW::vulkancore::DataBuffer::createIndexBuffer(device, sizeof(uint32_t) * submesh->mIndexCount, submesh->mSubMeshIndices.data());

			// Insert into map
			mSubMeshes.insert({ name, submesh });
		}

		//// 6. (Optional) If you want to support classic non-submesh drawing, create a single global index buffer
		//if (!mIndexDatas.empty()) {
		//	mIndexBuffer = VKFW::vulkancore::DataBuffer::createIndexBuffer(
		//		device,
		//		sizeof(uint32_t) * mIndexDatas.size(),
		//		mIndexDatas.data()
		//	);
		//}

		setVertexInputBindingDescriptions();
		setAttributeDescription();

	}

	void Model::loadBattleFireComponent(const std::string& path, const VKFW::Ref<VKFW::vulkancore::Device>& device) {
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);

		// 1. Read vertex count
		int vertexCount = 0;
		file.read(reinterpret_cast<char*>(&vertexCount), sizeof(int));
		if (!file) throw std::runtime_error("Failed to read vertex count");

		// 2. Read vertex data
		mBattleFireComponentVertexDatas.resize(vertexCount);
		file.read(reinterpret_cast<char*>(mBattleFireComponentVertexDatas.data()), sizeof(BattleFireComponentVertexData) * vertexCount);
		if (!file) throw std::runtime_error("Failed to read vertex data");

		//// 3. (Optional) You can also fill mPositions, mColors, mUVs if you want consistency with your OBJ loader
		//mPositions.clear();
		//mColors.clear();
		//mUVs.clear();
		//for (const auto& v : mBattleFireVertexDatas) {
		//	mPositions.push_back(v.mPosition.x);
		//	mPositions.push_back(v.mPosition.y);
		//	mPositions.push_back(v.mPosition.z);
		//	mColors.push_back(v.mColor.r);
		//	mColors.push_back(v.mColor.g);
		//	mColors.push_back(v.mColor.b);
		//	mUVs.push_back(v.mUV.x);
		//	mUVs.push_back(v.mUV.y);
		//}

		// 4. Create vertex buffer
		mVertexDataBuffer = VKFW::vulkancore::DataBuffer::createVertexBuffer(device, mBattleFireComponentVertexDatas.size() * sizeof(BattleFireComponentVertexData), mBattleFireComponentVertexDatas.data());

		// 5. Read submeshes until end of file
		mSubMeshes.clear();
		mIndexDatas.clear();
		mIndexBuffer.reset();

		while (file.peek() != EOF) {
			int nameLen = 0;
			file.read(reinterpret_cast<char*>(&nameLen), sizeof(int));
			if (!file || file.eof()) break;

			std::string name(nameLen, '\0');
			file.read(&name[0], nameLen);
			if (!file) break;

			int indexCount = 0;
			file.read(reinterpret_cast<char*>(&indexCount), sizeof(int));
			if (!file) break;

			std::vector<uint32_t> indices(indexCount);
			file.read(reinterpret_cast<char*>(indices.data()), sizeof(uint32_t) * indexCount);
			if (!file) break;

			// For global index buffer if needed (optional, for non-submesh drawing)
			mIndexDatas.insert(mIndexDatas.end(), indices.begin(), indices.end());

			// Create submesh
			SubMesh* submesh = new SubMesh;
			submesh->mIndexCount = indexCount;
			submesh->mSubMeshIndices = std::move(indices);

			// Create a Vulkan buffer for the submesh indices
			submesh->mSubMeshIndexBuffer = VKFW::vulkancore::DataBuffer::createIndexBuffer(device, sizeof(uint32_t) * submesh->mIndexCount, submesh->mSubMeshIndices.data());

			// Insert into map
			mSubMeshes.insert({ name, submesh });
		}

		//// 6. (Optional) If you want to support classic non-submesh drawing, create a single global index buffer
		//if (!mIndexDatas.empty()) {
		//	mIndexBuffer = VKFW::vulkancore::DataBuffer::createIndexBuffer(
		//		device,
		//		sizeof(uint32_t) * mIndexDatas.size(),
		//		mIndexDatas.data()
		//	);
		//}

		setVertexInputBindingDescriptions();
		setAttributeDescription();

	}

	void Model::setVertexInputBindingDescriptions() {
		if (!mVertexDatas.empty()) {
			// If vertex data is already set, use it
			bindingDes.resize(1);
			bindingDes[0].binding = 0;
			bindingDes[0].stride = sizeof(StaticMeshVertexData);
			bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Vertex input rate is per vertex
		}
		else if (!mBattleFireVertexDatas.empty()) {
			// If battle fire vertex data is set, use it
			bindingDes.resize(1);
			bindingDes[0].binding = 0;
			bindingDes[0].stride = sizeof(BattleFireMeshVertexData);
			bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Vertex input rate is per vertex
		}
		else if (!mBattleFireComponentVertexDatas.empty()) {
			// If battle fire component vertex data is set, use it
			bindingDes.resize(1);
			bindingDes[0].binding = 0;
			bindingDes[0].stride = sizeof(BattleFireComponentVertexData);
			bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Vertex input rate is per vertex
		}
		else {
			throw std::runtime_error("Error: No vertex data available to set binding descriptions.");
		}

	}

	std::vector<VkVertexInputBindingDescription> Model::getVertexInputBindingDescriptions() {

		return bindingDes;
	}


	void Model::setAttributeDescription() {
		if (!mVertexDatas.empty()) {
			// If vertex data is already set, use it
			attributeDes.resize(5);
			attributeDes[0].binding = 0;
			attributeDes[0].location = 0;
			attributeDes[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Position
			attributeDes[0].offset = offsetof(StaticMeshVertexData, mPosition);

			attributeDes[1].binding = 0;
			attributeDes[1].location = 1;
			attributeDes[1].format = VK_FORMAT_R32G32B32_SFLOAT; // Color
			attributeDes[1].offset = offsetof(StaticMeshVertexData, mColor);


			attributeDes[2].binding = 0;
			attributeDes[2].location = 2;
			attributeDes[2].format = VK_FORMAT_R32G32_SFLOAT; // UV
			attributeDes[2].offset = offsetof(StaticMeshVertexData, mUV);

			attributeDes[3].binding = 0;
			attributeDes[3].location = 3;
			attributeDes[3].format = VK_FORMAT_R32G32B32_SFLOAT; // Normal
			attributeDes[3].offset = offsetof(StaticMeshVertexData, mNormal);

			attributeDes[4].binding = 0;
			attributeDes[4].location = 4;
			attributeDes[4].format = VK_FORMAT_R32G32B32_SFLOAT; // Tangent
			attributeDes[4].offset = offsetof(StaticMeshVertexData, mTangent);
		}
		else if (!mBattleFireVertexDatas.empty()) {
			// If battle fire vertex data is set, use it
			attributeDes.resize(4);
			attributeDes[0].binding = 0;
			attributeDes[0].location = 0;
			attributeDes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Position
			attributeDes[0].offset = offsetof(BattleFireMeshVertexData, mPosition);

			attributeDes[1].binding = 0;
			attributeDes[1].location = 1;
			attributeDes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Texcoord
			attributeDes[1].offset = offsetof(BattleFireMeshVertexData, mTexcoord);

			attributeDes[2].binding = 0;
			attributeDes[2].location = 2;
			attributeDes[2].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Normal
			attributeDes[2].offset = offsetof(BattleFireMeshVertexData, mNormal);

			attributeDes[3].binding = 0;
			attributeDes[3].location = 3;
			attributeDes[3].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Tangent
			attributeDes[3].offset = offsetof(BattleFireMeshVertexData, mTangent);
		}
		else if (!mBattleFireComponentVertexDatas.empty()) {
			// If battle fire component vertex data is set, use it
			attributeDes.resize(3);
			attributeDes[0].binding = 0;
			attributeDes[0].location = 0;
			attributeDes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Position
			attributeDes[0].offset = offsetof(BattleFireComponentVertexData, mPosition);

			attributeDes[1].binding = 0;
			attributeDes[1].location = 1;
			attributeDes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Texcoord
			attributeDes[1].offset = offsetof(BattleFireComponentVertexData, mTexcoord);

			attributeDes[2].binding = 0;
			attributeDes[2].location = 2;
			attributeDes[2].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Normal
			attributeDes[2].offset = offsetof(BattleFireComponentVertexData, mNormal);
		}
		else {
			throw std::runtime_error("Error: No vertex data available to set attribute descriptions.");
		}


	}

	void Model::draw(const VKFW::Ref<VKFW::vulkancore::CommandBuffer>& cmdBuf) {
		cmdBuf->bindVertexBuffer(getVertexDataBuffer());
		if (!mSubMeshes.empty()) {
			// If there are submeshes, draw each submesh
			for (const auto& subMesh : mSubMeshes) {
				cmdBuf->bindIndexBuffer(subMesh.second->mSubMeshIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
				cmdBuf->drawIndexed(subMesh.second->mIndexCount, 1, 0, 0, 0);
			}
		}
		else {
			cmdBuf->bindIndexBuffer(getIndexBuffer()->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			cmdBuf->drawIndexed(getIndexCount(), 1, 0, 0, 0);
		}
	}


	std::vector<VkVertexInputAttributeDescription> Model::getAttributeDescriptions() {
		return attributeDes;
	}

	void Model::setVertexCount(int inVertexCount) {
		mVertexDatas.resize(inVertexCount);
	}

	void Model::setPosition(int inIndex, float inX, float inY, float inZ) {
		mVertexDatas[inIndex].mPosition.x = inX;
		mVertexDatas[inIndex].mPosition.y = inY;
		mVertexDatas[inIndex].mPosition.z = inZ;
	}

	void Model::setColor(int inIndex, float inR, float inG, float inB) {
		mVertexDatas[inIndex].mColor.r = inR;
		mVertexDatas[inIndex].mColor.g = inG;
		mVertexDatas[inIndex].mColor.b = inB;
	}

	void Model::setUV(int inIndex, float inU, float inV) {
		mVertexDatas[inIndex].mUV.x = inU;
		mVertexDatas[inIndex].mUV.y = inV;
	}

	void Model::setNormal(int inIndex, float inX, float inY, float inZ) {
		mVertexDatas[inIndex].mNormal.x = inX;
		mVertexDatas[inIndex].mNormal.y = inY;
	}
	void Model::setTangent(int inIndex, float inX, float inY, float inZ) {
		mVertexDatas[inIndex].mTangent.x = inX;
		mVertexDatas[inIndex].mTangent.y = inY;
	}
}