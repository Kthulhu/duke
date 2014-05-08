#include "ImageLoadUtils.hpp"

#include <duke/attributes/Attributes.hpp>
#include <duke/attributes/AttributeKeys.hpp>
#include <duke/filesystem/MemoryMappedFile.hpp>
#include <duke/filesystem/FsUtils.hpp>
#include <duke/gl/Textures.hpp>
#include <duke/image/FrameDescription.hpp>
#include <duke/imageio/DukeIO.hpp>
#include <duke/memory/Allocator.hpp>

#include <sstream>

using std::move;

namespace duke {

namespace {

AlignedMalloc alignedMalloc;

ReadFrameResult error(const std::string& error, ReadFrameResult& result) {
  result.error = error;
  result.status = IOResult::FAILURE;
  return move(result);
}

ReadFrameResult tryReader(const char* filename, const IIODescriptor* pDescriptor,
                          const attribute::Attributes& readOptions, const LoadCallback& callback,
                          ReadFrameResult&& result) {
  std::unique_ptr<IImageReader> pReader;
  if (pDescriptor->supports(IIODescriptor::Capability::READER_READ_FROM_MEMORY)) {
    MemoryMappedFile file(filename);
    if (!file) return error("unable to map file to memory", result);
    pReader.reset(pDescriptor->createMemoryReader(readOptions, file.pFileData, file.fileSize));
  } else {
    pReader.reset(pDescriptor->createFileReader(readOptions, filename));
  }
  return loadImage(pReader.get(), callback, move(result));
}

ReadFrameResult load(const char* pFilename, const char* pExtension, const attribute::Attributes& readOptions,
                     const LoadCallback& callback, ReadFrameResult&& result) {
  const auto& descriptors = IODescriptors::instance().findDescriptor(pExtension);
  if (descriptors.empty()) return error("no reader available", result);
  for (const IIODescriptor* pDescriptor : descriptors) {
    result = tryReader(pFilename, pDescriptor, readOptions, callback, move(result));
    if (result) return move(result);
  }
  return error("no reader succeeded, last message was : '" + result.error + "'", result);
}

}  // namespace

ReadFrameResult loadImage(IImageReader* pReader, const LoadCallback& callback, ReadFrameResult&& result) {
  CHECK(pReader);
  if (pReader->hasError()) return error(pReader->getError(), result);
  FrameData& frame = result.frame;
  if (!pReader->setup(frame)) return error(pReader->getError(), result);
  const void* pMapped = pReader->getMappedImageData();
  if (pMapped) {
    callback(frame, pMapped);
  } else {
    frame.pData = make_shared_memory<char>(frame.description.dataSize, alignedMalloc);
    pReader->readImageDataTo(frame.pData.get());
    if (pReader->hasError()) return error(pReader->getError(), result);
    callback(frame, frame.pData.get());
  }
  result.readerAttributes = pReader->moveAttributes();
  result.status = IOResult::SUCCESS;
  return move(result);
}

ReadFrameResult load(const attribute::Attributes& readOptions, const LoadCallback& callback, ReadFrameResult&& result) {
  const char* pFilename = attribute::getOrDie<attribute::File>(result.attributes());
  if (!pFilename) return error("no filename", result);
  const char* pExtension = fileExtension(pFilename);
  if (!pExtension) return error("no extension", result);
  return load(pFilename, pExtension, readOptions, callback, move(result));
}

ReadFrameResult load(const char* pFilename, Texture& texture) {
  CHECK(pFilename);
  ReadFrameResult result;
  attribute::set<attribute::File>(result.attributes(), pFilename);
  return load({}, [&](FrameData& frame, const void* pVolatileData) {
                    const auto bound = texture.scope_bind_texture();
                    texture.initialize(frame.description, pVolatileData);
                  },
              std::move(result));
}

} /* namespace duke */
