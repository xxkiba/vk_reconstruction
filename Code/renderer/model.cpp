#include "model.h"
#include <glm/gtc/type_ptr.hpp>              
#include <glm/gtc/quaternion.hpp> 
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include "tiny_gltf.h"

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

	void Model::loadGltfModel(const std::string& path, const VKFW::Ref<VKFW::vulkancore::Device>& device) {
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF loader;
		std::string err, warn;
		// We don't need image data, set a no-op loader to satisfy tinygltf
		loader.SetImageLoader([](tinygltf::Image*, const int, std::string*, std::string*,
			int, int, const unsigned char*, int, void*) { return true; }, nullptr);

		bool loaded = (path.size() >= 4 && path.substr(path.size() - 4) == ".glb")
			? loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path)
			: loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);

		if (!loaded)
			throw std::runtime_error("Failed to load glTF: " + err);

		mBattleFireVertexDatas.clear();
		mSubMeshes.clear();
		mIndexDatas.clear();
		mIndexBuffer.reset();

		for (int meshIdx = 0; meshIdx < (int)gltfModel.meshes.size(); ++meshIdx) {
			const auto& mesh = gltfModel.meshes[meshIdx];

			// Collect node transform for this mesh
			glm::mat4 nodeMatrix(1.0f);
			for (const auto& node : gltfModel.nodes) {
				if (node.mesh != meshIdx) continue;
				if (!node.matrix.empty()) {
					nodeMatrix = glm::make_mat4(node.matrix.data());
				} else {
					if (!node.translation.empty())
						nodeMatrix = glm::translate(nodeMatrix, glm::vec3(
							(float)node.translation[0], (float)node.translation[1], (float)node.translation[2]));
					if (!node.rotation.empty()) {
						// gltf quaternion is (x,y,z,w); glm::quat ctor is (w,x,y,z)
						glm::quat q((float)node.rotation[3], (float)node.rotation[0],
									(float)node.rotation[1], (float)node.rotation[2]);
						nodeMatrix *= glm::mat4_cast(q);
					}
					if (!node.scale.empty())
						nodeMatrix = glm::scale(nodeMatrix, glm::vec3(
							(float)node.scale[0], (float)node.scale[1], (float)node.scale[2]));
				}
				break;
			}
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(nodeMatrix)));

			for (size_t primIdx = 0; primIdx < mesh.primitives.size(); ++primIdx) {
				const auto& primitive = mesh.primitives[primIdx];

				auto getAttr = [&](const std::string& name) -> int {
					auto it = primitive.attributes.find(name);
					return it != primitive.attributes.end() ? it->second : -1;
				};

				int posIdx  = getAttr("POSITION");
				int normIdx = getAttr("NORMAL");
				int uvIdx   = getAttr("TEXCOORD_0");
				int tanIdx  = getAttr("TANGENT");

				if (posIdx < 0) continue;

				int vertexCount  = (int)gltfModel.accessors[posIdx].count;
				int vertexOffset = (int)mBattleFireVertexDatas.size();

				// Helper: get typed pointer into accessor element i, respecting byteStride
				auto ptr = [&](int accIdx, size_t elemSize, int i) -> const uint8_t* {
					const auto& acc  = gltfModel.accessors[accIdx];
					const auto& view = gltfModel.bufferViews[acc.bufferView];
					size_t stride    = view.byteStride ? view.byteStride : elemSize;
					return gltfModel.buffers[view.buffer].data.data()
						   + view.byteOffset + acc.byteOffset + i * stride;
				};

				// Read all vertices
				for (int i = 0; i < vertexCount; ++i) {
					BattleFireMeshVertexData v{};

					const float* p = reinterpret_cast<const float*>(ptr(posIdx, sizeof(float) * 3, i));
					v.mPosition = nodeMatrix * glm::vec4(p[0], p[1], p[2], 1.0f);

					if (normIdx >= 0) {
						const float* n = reinterpret_cast<const float*>(ptr(normIdx, sizeof(float) * 3, i));
						glm::vec3 norm = glm::normalize(normalMatrix * glm::vec3(n[0], n[1], n[2]));
						v.mNormal = glm::vec4(norm, 0.0f);
					}

					if (uvIdx >= 0) {
						const float* uv = reinterpret_cast<const float*>(ptr(uvIdx, sizeof(float) * 2, i));
						v.mTexcoord = glm::vec4(uv[0], uv[1], 0.0f, 0.0f);
					}

					if (tanIdx >= 0) {
						const float* t = reinterpret_cast<const float*>(ptr(tanIdx, sizeof(float) * 4, i));
						glm::vec3 tan = glm::normalize(glm::mat3(nodeMatrix) * glm::vec3(t[0], t[1], t[2]));
						v.mTangent = glm::vec4(tan, t[3]); // t[3] = handedness (+1/-1)
					}

					mBattleFireVertexDatas.push_back(v);
				}

				// Read indices
				std::vector<uint32_t> indices;
				if (primitive.indices >= 0) {
					const auto& idxAcc  = gltfModel.accessors[primitive.indices];
					const auto& idxView = gltfModel.bufferViews[idxAcc.bufferView];
					const uint8_t* data = gltfModel.buffers[idxView.buffer].data.data()
						                  + idxView.byteOffset + idxAcc.byteOffset;
					int count = (int)idxAcc.count;
					indices.resize(count);

					switch (idxAcc.componentType) {
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
							const uint32_t* src = reinterpret_cast<const uint32_t*>(data);
							for (int i = 0; i < count; ++i) indices[i] = src[i] + vertexOffset;
							break;
						}
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
							const uint16_t* src = reinterpret_cast<const uint16_t*>(data);
							for (int i = 0; i < count; ++i) indices[i] = src[i] + vertexOffset;
							break;
						}
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
							for (int i = 0; i < count; ++i) indices[i] = data[i] + vertexOffset;
							break;
						}
					}
				}

				// Compute tangents (Lengyel) if glTF doesn't provide them
				if (tanIdx < 0 && normIdx >= 0 && uvIdx >= 0 && !indices.empty()) {
					std::vector<glm::vec3> tanAcc(vertexCount, glm::vec3(0.0f));
					for (size_t i = 0; i + 2 < indices.size(); i += 3) {
						int i0 = (int)indices[i]     - vertexOffset;
						int i1 = (int)indices[i + 1] - vertexOffset;
						int i2 = (int)indices[i + 2] - vertexOffset;
						auto& v0 = mBattleFireVertexDatas[vertexOffset + i0];
						auto& v1 = mBattleFireVertexDatas[vertexOffset + i1];
						auto& v2 = mBattleFireVertexDatas[vertexOffset + i2];
						glm::vec3 e1  = glm::vec3(v1.mPosition) - glm::vec3(v0.mPosition);
						glm::vec3 e2  = glm::vec3(v2.mPosition) - glm::vec3(v0.mPosition);
						float du1 = v1.mTexcoord.x - v0.mTexcoord.x, dv1 = v1.mTexcoord.y - v0.mTexcoord.y;
						float du2 = v2.mTexcoord.x - v0.mTexcoord.x, dv2 = v2.mTexcoord.y - v0.mTexcoord.y;
						float denom = du1 * dv2 - du2 * dv1;
						if (glm::abs(denom) < 1e-8f) continue;
						glm::vec3 T = (1.0f / denom) * (dv2 * e1 - dv1 * e2);
						tanAcc[i0] += T; tanAcc[i1] += T; tanAcc[i2] += T;
					}
					for (int i = 0; i < vertexCount; ++i) {
						auto& v    = mBattleFireVertexDatas[vertexOffset + i];
						glm::vec3 N = glm::vec3(v.mNormal);
						glm::vec3 T = glm::normalize(tanAcc[i] - glm::dot(N, tanAcc[i]) * N);
						v.mTangent  = glm::vec4(T, 0.0f);
					}
				}

				// Create submesh
				mIndexDatas.insert(mIndexDatas.end(), indices.begin(), indices.end());

				std::string submeshName = mesh.name;
				if (mesh.primitives.size() > 1)
					submeshName += "_" + std::to_string(primIdx);

				SubMesh* submesh         = new SubMesh;
				submesh->mIndexCount     = (int)indices.size();
				submesh->mSubMeshIndices = std::move(indices);
				submesh->mSubMeshIndexBuffer = VKFW::vulkancore::DataBuffer::createIndexBuffer(
					device, sizeof(uint32_t) * submesh->mIndexCount, submesh->mSubMeshIndices.data());
				mSubMeshes.insert({ submeshName, submesh });
			}
		}

		mVertexDataBuffer = VKFW::vulkancore::DataBuffer::createVertexBuffer(
			device, mBattleFireVertexDatas.size() * sizeof(BattleFireMeshVertexData),
			mBattleFireVertexDatas.data());

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