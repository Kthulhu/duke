#include "LoadedPboCache.hpp"

#include "duke/engine/cache/LoadedImageCache.hpp"
#include "duke/image/ImageUtils.hpp"

namespace duke {

bool LoadedPboCache::get(const LoadedImageCache& imageCache, const MediaFrameReference& mfr, PboPackedFrame& pbo) {
  auto pFound = m_Map.find(mfr);
  if (pFound == m_Map.end()) {
    FrameData frame;
    const bool inCache = imageCache.get(mfr, frame);
    const auto dataSize = frame.getData().size();
    if (!inCache || dataSize == 0) return false;
    evictOneIfNecessary();
    auto pSharedPbo = m_PboPool.get(dataSize);
    {  // transfer buffer
      const auto target = pSharedPbo->target;
      const auto offset = 0;
      const auto length = dataSize;
      const auto access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
      const auto data = frame.getData();
      auto pboBound = pSharedPbo->scope_bind_buffer();
      void* destPtr = glMapBufferRange(target, offset, length, access);
      memcpy(destPtr, data.begin(), data.size());
      glUnmapBuffer(target);
    }
    PboPackedFrame pboPackedFrame(frame.getDescription());
    pboPackedFrame.pPbo = std::move(pSharedPbo);
    pFound = m_Map.insert({mfr, std::move(pboPackedFrame)}).first;
  }
  pbo = pFound->second;
  moveFront(mfr);
  return true;
}

void LoadedPboCache::moveFront(const MediaFrameReference& mfr) {
  auto pFound = std::find(begin(m_Fifo), end(m_Fifo), mfr);
  if (pFound != end(m_Fifo))
    std::swap(*begin(m_Fifo), *pFound);
  else
    m_Fifo.insert(begin(m_Fifo), mfr);
}

void LoadedPboCache::evictOneIfNecessary() {
  if (m_Map.size() >= m_MaxCount) {
    const MediaFrameReference oldest = m_Fifo.back();
    m_Map.erase(m_Map.find(oldest));
    m_Fifo.pop_back();
  }
}

} /* namespace duke */
