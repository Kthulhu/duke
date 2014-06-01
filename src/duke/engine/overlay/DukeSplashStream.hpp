#pragma once

#include "IOverlay.hpp"
#include "duke/engine/rendering/GlyphRenderer.hpp"
#include "duke/animation/Animation.hpp"

namespace duke {

class DukeSplashStream : public duke::IOverlay {
 public:
  DukeSplashStream();
  virtual ~DukeSplashStream();
  virtual void render(const Context&) const;

 private:
  AnimationData m_RightAlpha;
  AnimationData m_LeftAlpha;
  AnimationData m_LeftPos;
};

} /* namespace duke */
