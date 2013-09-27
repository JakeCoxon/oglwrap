#ifndef OGLWRAP_SHAPES_ANIMATED_MESH_SKINNING_INL_HPP_
#define OGLWRAP_SHAPES_ANIMATED_MESH_SKINNING_INL_HPP_

#include "animatedMesh.hpp"

namespace oglwrap {

void AnimatedMesh::mapBones() {
  for(size_t entry = 0; entry < entries.size(); entry++) {
    const aiMesh* pMesh = scene->mMeshes[entry];

    for(unsigned i = 0; i < pMesh->mNumBones; i++) {
      std::string boneName(pMesh->mBones[i]->mName.data);
      size_t boneIndex = 0;

      // Search for this bone in the BoneMap
      if(bone_mapping.find(boneName) == bone_mapping.end()) {
        // Allocate an index for the new bone
        boneIndex = num_bones++;
        bone_info.push_back(BoneInfo());
        bone_info[boneIndex].bone_offset = convertMatrix(pMesh->mBones[i]->mOffsetMatrix);
        bone_mapping[boneName] = boneIndex;
      }
    }
  }
}

const aiNodeAnim* AnimatedMesh::getRootBone(const aiNode* node, const aiScene* anim) {
  std::string nodeName(node->mName.data);

  const aiAnimation* animation = anim->mAnimations[0];
  const aiNodeAnim* nodeAnim = findNodeAnim(animation, nodeName);

  if(nodeAnim) {
    if(root_bone.empty()) {
      root_bone = nodeName;
    } else {
      if(root_bone != nodeName) {
        throw std::runtime_error(
          "Animation error: the animated skeletons have different root bones."
        );
      }
    }
    return nodeAnim;
  } else {
    for(unsigned i = 0; i < node->mNumChildren; i++) {
      auto childsReturn = getRootBone(node->mChildren[i], anim);
      if(childsReturn) {
        return childsReturn;
      }
    }
  }

  return nullptr;
}

template <class Index_t>
void AnimatedMesh::loadBones() {

  const size_t per_attrib_size = sizeof(VertexBoneData_PerAttribute<Index_t>);

  per_mesh_attrib_max.resize(entries.size());

  for(size_t entry = 0; entry < entries.size(); entry++) {
    std::vector<VertexBoneData<Index_t>> vertices;
    const aiMesh* pMesh = scene->mMeshes[entry];
    vertices.resize(pMesh->mNumVertices);

    // -------======{[ Create the bone ID's and weights data ]}======-------

    for(unsigned i = 0; i < pMesh->mNumBones; i++) {
      std::string boneName(pMesh->mBones[i]->mName.data);
      size_t boneIndex = bone_mapping[boneName];

      for(unsigned j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
        unsigned vertexID = pMesh->mBones[i]->mWeights[j].mVertexId;
        float weight = pMesh->mBones[i]->mWeights[j].mWeight;
        vertices[vertexID].AddBoneData(boneIndex, weight);
      }
    }

    // -------======{[ Upload the bone data ]}======-------

    entries[entry].vao.bind();
    vertex_bone_data_buffers[entry].bind();

    // I can't just upload to the buffer with .data(), as bones aren't stored in a continuous buffer,
    // and it is an array of not fixed sized arrays, but OpenGL needs it in fix sized parts.

    // Get the current number of max bone attributes.
    unsigned char& current_attrib_max = per_mesh_attrib_max[entry];
    for(size_t i = 0; i < vertices.size(); i++) {
      if(vertices[i].data.size() > current_attrib_max) {
        current_attrib_max = vertices[i].data.size();
      }
    }

    if(current_attrib_max > max_bone_attrib_num) {
      max_bone_attrib_num = current_attrib_max;
    }

    size_t per_vertex_size = current_attrib_max * per_attrib_size;
    size_t buffer_size = vertices.size() * per_vertex_size;

#if OGLWRAP_PORTABILITY_MODE // Don't use mapping in portability mode

    // Upload the bones data into a continuous buffer then upload that to OpenGL.
    GLbyte* data = new GLbyte[buffer_size];
    GLintptr offset = 0;
    for(size_t i = 0; i < vertices.size(); i++) {
      size_t curr_size = vertices[i].data.size() * per_attrib_size;

      // Copy the bone data
      memcpy(
        data + offset, // destination
        vertices[i].data.data(),  // source
        curr_size // length
      );

      // Zero out all the remaining memory. Remember a
      // bone with a 0.0f weight doesn't have any influence
      if(per_vertex_size > curr_size) {
        memset(
          data + offset + curr_size, // memory place
          0, // value
          per_vertex_size - curr_size // length
        );
      }

      offset += per_vertex_size;
    }

    // upload
    vertex_bone_data_buffers[entry].data(buffer_size, data);
    delete[] data;

#else // Upload the bones data in continuous, fix-sized parts using mapping.

    // First we have to allocate the buffer's storage.
    vertex_bone_data_buffers[entry].data(buffer_size, (void*)0);

    {
      // The buffer gets unmapped when it's lifetime ends
      ArrayBuffer::Map bones_buffer_map(BufferMapAccess::Write);
      GLintptr offset = 0;
      for(size_t i = 0; i < vertices.size(); i++) {
        size_t curr_size = vertices[i].data.size() * per_attrib_size;

        // Copy the bone data
        memcpy(
          (GLbyte*)bones_buffer_map.data() + offset, // destination
          vertices[i].data.data(),  // source
          curr_size // length
        );

        // Zero out all the remaining memory. Remember a
        // bone with a 0.0f weight doesn't have any influence
        if(per_vertex_size > curr_size) {
          memset(
            (GLbyte*)bones_buffer_map.data() + offset + curr_size, // memory place
            0, // value
            per_vertex_size - curr_size // length
          );
        }

        offset += per_vertex_size;
      }
    }

#endif // OGLWRAP_PORTABILITY_MODE

  }

  // Unbind our things, so they won't be modified from outside
  VertexArray::Unbind();
  ArrayBuffer::Unbind();
}

void AnimatedMesh::create_bones_data() {
  mapBones();

  if(num_bones < UCHAR_MAX) {
    loadBones<unsigned char>();
  } else if(num_bones < USHRT_MAX) {
    loadBones<unsigned short>();
  } else { // more than 65535 bones? WTF???
    loadBones<unsigned int>();
  }
}

template <class Index_t>
void AnimatedMesh::shader_plumb_bones(DataType idx_t, LazyVertexAttribArray boneIDs, LazyVertexAttribArray boneWeights) {
  const size_t per_attrib_size = sizeof(VertexBoneData_PerAttribute<Index_t>);

  for(size_t entry = 0; entry < entries.size(); entry++) {

    entries[entry].vao.bind();
    vertex_bone_data_buffers[entry].bind();
    unsigned char current_attrib_max = per_mesh_attrib_max[entry];

    for(unsigned char boneAttribSet = 0; boneAttribSet < current_attrib_max; boneAttribSet++) {
      const size_t stride = current_attrib_max * per_attrib_size;

      intptr_t baseOffset = boneAttribSet * per_attrib_size;
      intptr_t weightOffset = baseOffset + 4 * sizeof(Index_t);

      boneIDs[boneAttribSet].setup(4, idx_t, stride, (const void*)baseOffset).enable();
      boneWeights[boneAttribSet].setup(4, DataType::Float, stride, (const void*)weightOffset).enable();
    }

    // static setup the VertexArrays that aren't enabled, to all zero.
    // Remember (0, 0, 0, 1) is the default, which isn't what we want.
    for(int i = current_attrib_max; i < max_bone_attrib_num; i++) {
      boneIDs[i].static_setup(glm::ivec4(0, 0, 0, 0));
      boneWeights[i].static_setup(glm::vec4(0, 0, 0, 0));
    }
  }

  // Unbind our things, so they won't be modified from outside
  VertexArray::Unbind();
  ArrayBuffer::Unbind();
}

size_t AnimatedMesh::get_num_bones() {

  // If loadBones hasn't been called yet, than have to create
  // the bones data first to know the number of bones.
  if(per_mesh_attrib_max.size() == 0) {
    create_bones_data();
  }

  return num_bones;
}

size_t AnimatedMesh::get_bone_attrib_num() {

  // If loadBones hasn't been called yet, than have to create
  // the bones data first to know max_bone_attrib_num.
  if(per_mesh_attrib_max.size() == 0) {
    create_bones_data();
  }

  return max_bone_attrib_num;
}

void AnimatedMesh::setup_bones(LazyVertexAttribArray boneIDs, LazyVertexAttribArray boneWeights) {

  if(is_setup_bones) {
    throw std::logic_error("AnimatedMesh::setup_bones is called multiply times on the same object");
  } else {
    is_setup_bones = true;
  }

  // If the bones data hasn't been created yet, than call the function to do it.
  if(per_mesh_attrib_max.size() == 0) {
    create_bones_data();
  }

  if(num_bones < UCHAR_MAX) {
    shader_plumb_bones<unsigned char>(DataType::UnsignedByte, boneIDs, boneWeights);
  } else if(num_bones < USHRT_MAX) {
    shader_plumb_bones<unsigned short>(DataType::UnsignedShort, boneIDs, boneWeights);
  } else { // more than 65535 bones? WTF???
    shader_plumb_bones<unsigned int>(DataType::UnsignedInt, boneIDs, boneWeights);
  }
}

} // namespace oglwrap

#endif // OGLWRAP_SHAPES_ANIMATED_MESH_SKINNING_INL_HPP_
