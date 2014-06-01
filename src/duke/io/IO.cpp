#include "IO.hpp"

#include <stdexcept>

#include <cassert>

using namespace std;

namespace duke {

bool IODescriptors::registerDescriptor(IIODescriptor* pDescriptor) {
  assert(pDescriptor);
  m_Descriptors.push_back(unique_ptr<IIODescriptor>(pDescriptor));
  const bool generalPurpose = pDescriptor->supports(IIODescriptor::Capability::READER_GENERAL_PURPOSE);
  for (const string& extension : pDescriptor->getSupportedExtensions())
    if (generalPurpose)
      m_ExtensionToDescriptors[extension].push_back(pDescriptor);
    else
      m_ExtensionToDescriptors[extension].push_front(pDescriptor);
  return true;
}

const deque<IIODescriptor*>& IODescriptors::findDescriptor(const char* extension) const {
  const auto pFound = m_ExtensionToDescriptors.find(extension);
  if (pFound != m_ExtensionToDescriptors.end()) return pFound->second;
  static const deque<IIODescriptor*> EmptyVector;
  return EmptyVector;
}

bool IODescriptors::isSupported(const char* extension) const {
  return m_ExtensionToDescriptors.find(extension) != m_ExtensionToDescriptors.end();
}

IODescriptors& IODescriptors::instance() {
  static IODescriptors descriptors;
  return descriptors;
}

}  // namespace duke
