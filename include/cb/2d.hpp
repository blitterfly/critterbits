#pragma once
#ifndef CB2D_HPP
#define CB2D_HPP

#include <vector>

#include "coord.hpp"

namespace Critterbits {
inline bool AabbCollision(const CB_Rect & rect1, const CB_Rect & rect2) {
    return (rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x && rect1.y < rect2.y + rect2.h &&
            rect1.h + rect1.y > rect2.y);
}

inline float Lerp(float start, float end, float percent) {
  return start + percent * (end - start);
}

inline CB_Point Lerp2D(const CB_Point & start, const CB_Point & end, float percent) {
  return start + percent * (end - start);
}

inline float QuadEaseIn(float start, float end, float percent) {
  return (end - start) * percent * percent + start;
}

inline CB_Point QuadEaseIn2D(const CB_Point & start, const CB_Point & end, float percent) {
  return (end - start) * percent * percent + start;
}	

class RectRegionCombiner {
  public:
    std::vector<CB_Rect> regions;

    RectRegionCombiner(){};
    void Combine();
};
}
#endif