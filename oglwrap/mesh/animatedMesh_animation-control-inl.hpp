#ifndef OGLWRAP_MESH_ANIMATED_MESH_ANIMATION_CONTROL_INL_HPP_
#define OGLWRAP_MESH_ANIMATED_MESH_ANIMATION_CONTROL_INL_HPP_

#include "animatedMesh.hpp"
#include "animInfo.hpp"

namespace oglwrap {

inline void AnimatedMesh::addAnimation(const std::string& filename,
                                       const std::string& anim_name,
                                       unsigned flags,
                                       float speed) {
   if(anims_.canFind(anim_name)) {
      std::string err = "Animation name '" + anim_name + "' isn't unique for '" + filename + "'";
      throw std::runtime_error(err);
   }
   size_t idx = anims_.data.size();
   anims_.names[anim_name] = idx;
   anims_.data.push_back(AnimInfo());
   anims_[idx].name = anim_name;
   anims_[idx].importer = new Assimp::Importer();
   anims_[idx].handle = anims_[idx].importer->ReadFile(filename, aiProcess_Debone);
   if(!anims_[idx].handle) {
      throw std::runtime_error("Error parsing " + filename
                               + " : " + anims_[idx].importer->GetErrorString());
   }

   auto node = getRootBone(scene_->mRootNode, anims_[idx].handle);
   if(!node) {
      throw std::runtime_error(
         "Animation error: The mesh's skeleton, and the animated skeleton '" +
         anim_name + "' doesn't have a single bone in common.");
   }

   aiVector3D v = node->mPositionKeys[0].mValue;
   anims_[idx].start_offset = glm::vec3(v.x, v.y, v.z);

   v = node->mPositionKeys[node->mNumPositionKeys - 1].mValue;
   anims_[idx].end_offset =  glm::vec3(v.x, v.y, v.z);

   anims_[idx].flags = flags;
   anims_[idx].speed = speed;
}

inline void AnimatedMesh::setDefaultAnimation(const std::string& anim_name,
                                              float default_transition_time) {
   if(!anims_.canFind(anim_name)) {
      throw std::invalid_argument(
         "Tried to set default animation to '" + anim_name + "', "
         "but the AnimatedMesh doesn't have an animation with that name"
      );
   }
   anim_meta_info_.default_idx = anims_.names[anim_name];
   anim_meta_info_.default_transition_time = default_transition_time;
   if(!(anims_[anim_meta_info_.default_idx].flags & AnimFlag::Repeat)) {
      throw std::invalid_argument(
         "Tried to set a default animation that didn't have the "
         "repeat flag, but the default animation must be a cycle."
      );
   }
}

inline void AnimatedMesh::changeAnimation(size_t anim_idx,
                                          float current_time,
                                          float transition_time,
                                          unsigned flags,
                                          float speed) {
   bool was_last_invalid = (last_anim_.handle == nullptr);

   last_anim_ = current_anim_;

   current_anim_.idx = anim_idx;
   current_anim_.handle = anims_[anim_idx].handle;
   current_anim_name_ = anims_[anim_idx].name;

   if(flags & AnimFlag::Backwards) {
      current_anim_.offset = anims_[anim_idx].end_offset;
   } else {
      current_anim_.offset = anims_[anim_idx].start_offset;
   }
   if(flags & AnimFlag::Mirrored) {
      current_anim_.offset *= -1;
   }

   last_anim_.offset = current_anim_.offset;

   if(speed > 0.0f) {
      current_anim_.speed = speed;
      current_anim_.flags = flags;
   } else {
      current_anim_.speed = -speed;
      current_anim_.flags = flags ^ AnimFlag::Backwards;
   }

   if(was_last_invalid) {
     last_anim_ = current_anim_;
     last_anim_.offset = glm::vec3();
   }

   // Meta animation data
   anim_meta_info_.transition_time = transition_time;
   anim_meta_info_.last_period_time = current_time - anim_meta_info_.end_of_last_anim;
   anim_meta_info_.end_of_last_anim = current_time;
}

inline void AnimatedMesh::setCurrentAnimation(const std::string& anim_name,
                                              float current_time,
                                              float transition_time,
                                              unsigned flags,
                                              float speed) {
   if((anim_meta_info_.end_of_last_anim + anim_meta_info_.transition_time) <= current_time
         && (current_anim_.flags & AnimFlag::Interruptable)) {
      if(!anims_.canFind(anim_name)) {
         throw std::invalid_argument(
            "Tried to set current animation to '" + anim_name + "', "
            "but the AnimatedMesh doesn't have an animation with name"
         );
      }
      size_t anim_idx = anims_.names[anim_name];
      if(current_anim_.handle != anims_[anim_idx].handle) {
         forceCurrentAnimation(anim_name, current_time, transition_time, flags, speed);
      }
   }
}

inline void AnimatedMesh::forceCurrentAnimation(const std::string& anim_name,
                                                float current_time,
                                                float transition_time,
                                                unsigned flags,
                                                float speed) {
   if(!anims_.canFind(anim_name)) {
      throw std::invalid_argument(
         "Tried to set current animation to '" + anim_name + "', "
         "but the AnimatedMesh doesn't have an animation with name"
      );
   }
   size_t anim_idx = anims_.names[anim_name];
   if(fabs(speed) < 1e-5) {
      changeAnimation(
         anim_idx,
         current_time,
         transition_time,
         flags,
         anims_[anim_idx].speed
      );
   } else {
      changeAnimation(
         anim_idx,
         current_time,
         transition_time,
         flags,
         speed
      );
   }
}

inline void AnimatedMesh::setCurrentAnimation(const std::string& anim_name,
                                              float current_time,
                                              float transition_time,
                                              float speed) {
  if((anim_meta_info_.end_of_last_anim + anim_meta_info_.transition_time) <= current_time
         && (current_anim_.flags & AnimFlag::Interruptable)
     ) {
      if(!anims_.canFind(anim_name)) {
        throw std::invalid_argument(
          "Tried to set current animation to '" + anim_name + "', "
          "but the AnimatedMesh doesn't have an animation with name"
        );
      }
      size_t anim_idx = anims_.names[anim_name];
      if(current_anim_.handle != anims_[anim_idx].handle) {
         forceCurrentAnimation(anim_name, current_time, transition_time, speed);
      }
   }
}

inline void AnimatedMesh::forceCurrentAnimation(const std::string& anim_name,
                                                float current_time,
                                                float transition_time,
                                                float speed) {
  if(!anims_.canFind(anim_name)) {
      throw std::invalid_argument(
         "Tried to set current animation to '" + anim_name + "', "
         "but the AnimatedMesh doesn't have an animation with name"
      );
   }
   size_t anim_idx = anims_.names[anim_name];

   if(fabs(speed) < 1e-5) {
      changeAnimation(
         anim_idx,
         current_time,
         transition_time,
         anims_[anim_idx].flags,
         anims_[anim_idx].speed
      );
   } else {
      changeAnimation(
         anim_idx,
         current_time,
         transition_time,
         anims_[anim_idx].flags,
         speed
      );
   }
}

inline void AnimatedMesh::setAnimToDefault(float current_time) {
  if(current_anim_.flags & AnimFlag::Interruptable &&
      current_anim_.handle != anims_[anim_meta_info_.default_idx].handle) {
    forceAnimToDefault(current_time);
  }
}

inline void AnimatedMesh::forceAnimToDefault(float current_time) {
  changeAnimation(
     anim_meta_info_.default_idx,
     current_time,
     anim_meta_info_.default_transition_time,
     anims_[anim_meta_info_.default_idx].flags,
     anims_[anim_meta_info_.default_idx].speed
  );
}

inline void AnimatedMesh::animationEnded(float current_time) {
   if(anim_ended_callback_ == nullptr) {
      forceAnimToDefault(current_time);
   } else {
      bool use_default_flags = true;
      float transition_time = 0.1f, speed = 0.0f;
      unsigned flags = AnimFlag::None;
      std::string new_anim = (*anim_ended_callback_)(
         getCurrentAnimation(),
         &transition_time,
         &use_default_flags,
         &flags,
         &speed
      );
      if(anims_.canFind(new_anim)) {
         if(use_default_flags) {
            forceCurrentAnimation(new_anim, current_time, transition_time, speed);
         } else {
            forceCurrentAnimation(new_anim, current_time, transition_time, flags, speed);
         }
      } else {
         forceAnimToDefault(current_time);
      }
   }
}

inline glm::vec2 AnimatedMesh::offsetSinceLastFrame() {
   auto ret = current_anim_.offset - last_anim_.offset;
   last_anim_.offset = current_anim_.offset;
   return glm::vec2(ret.x, ret.z);
}

} // namespace oglwrap

#endif // OGLWRAP_MESH_ANIMATED_MESH_ANIMATION_CONTROL_INL_HPP_
