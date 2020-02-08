// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs

#include <RWPly.hxx>

#if defined(HAVE_TINYPLY)

// Be sure to include these files before tinyply header, due to possible throw/try/catch and constexpr being redefined
#include <Standard_Macro.hxx>
#include <Standard_DefineException.hxx>

#include <Message_ProgressSentry.hxx>
#include <NCollection_Vector.hxx>
#include <OSD_File.hxx>
#include <OSD_OpenFile.hxx>

#include <tinyply.h>

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace tinyply;

template<typename T>
struct tuple2 { T x, y; };
template<typename T>
struct tuple3 { T x, y, z; };

template<typename T>
void HandleVertices(std::shared_ptr<PlyData> theData, opencascade::handle<Poly_Triangulation> thePoly) {
  const size_t numBytes = theData->buffer.size_bytes();
  std::vector<tuple3<T>> vec(theData->count);
  std::memcpy(vec.data(), theData->buffer.get(), numBytes);

  int i = 0;
  for (auto node : vec) {
    thePoly->ChangeNode(i + 1) = gp_Pnt(node.x, node.y, node.z);
    i++;
  }
}

template<typename T>
void HandleTexCoords(std::shared_ptr<PlyData> theData, opencascade::handle<Poly_Triangulation> thePoly) {
  const size_t numBytes = theData->buffer.size_bytes();
  std::vector<tuple2<T>> vec(theData->count);
  std::memcpy(vec.data(), theData->buffer.get(), numBytes);

  int i = 0;
  for (auto node : vec) {
    thePoly->ChangeUVNodes()(i + 1) = gp_Pnt2d(node.x, node.y);
    i++;
  }
}

template<typename T>
void HandleFaces(std::shared_ptr<PlyData> theData, opencascade::handle<Poly_Triangulation> thePoly) {
  const size_t numBytes = theData->buffer.size_bytes();
  std::vector<tuple3<T>> vec(theData->count);
  std::memcpy(vec.data(), theData->buffer.get(), numBytes);

  int i = 0;
  for (auto node : vec) {
    thePoly->ChangeTriangle(i + 1) = Poly_Triangle(node.x + 1, node.y + 1, node.z + 1);
    i++;
  }
}

opencascade::handle<Poly_Triangulation> RWPly::ReadFile(const Standard_CString theFile,
                                                        const opencascade::handle<Message_ProgressIndicator> &theProgress) {
  std::ifstream ss(theFile, std::ios::binary);
  if (ss.fail()) return nullptr;

  try {
    PlyFile file;
    std::shared_ptr<PlyData> vertices, faces, texcoords;

    file.parse_header(ss);
    vertices = file.request_properties_from_element("vertex", {"x", "y", "z"});
    if Standard_IF_CONSTEXPR(false) {
      texcoords = file.request_properties_from_element("vertex", {"u", "v"});
    }
    faces = file.request_properties_from_element("face", {"vertex_indices"}, 3);

    opencascade::handle<Poly_Triangulation>
        aPoly = new Poly_Triangulation(vertices->count, faces->count, Standard_False /*texcoords->count > 0*/);

    file.read(ss);

    switch (vertices->t) {
      case tinyply::Type::FLOAT32: {
        HandleVertices<float>(vertices, aPoly);
        break;
      }
      case tinyply::Type::FLOAT64: {
        HandleVertices<double>(vertices, aPoly);
        break;
      }
      default: {
        return nullptr;
      }
    }

    if Standard_IF_CONSTEXPR(false && texcoords->count > 0) {
      switch (texcoords->t) {
        case tinyply::Type::FLOAT32: {
          HandleTexCoords<float>(texcoords, aPoly);
          break;
        }
        case tinyply::Type::FLOAT64: {
          HandleTexCoords<double>(texcoords, aPoly);
          break;
        }
        default: {
          return nullptr;
        }
      }
    }

    switch (faces->t) {
      case tinyply::Type::INT8: {
        HandleFaces<int8_t>(faces, aPoly);
        break;
      }
      case tinyply::Type::UINT8: {
        HandleFaces<uint8_t>(faces, aPoly);
        break;
      }
      case tinyply::Type::INT16: {
        HandleFaces<int16_t>(faces, aPoly);
        break;
      }
      case tinyply::Type::UINT16: {
        HandleFaces<uint16_t>(faces, aPoly);
        break;
      }
      case tinyply::Type::INT32: {
        HandleFaces<int32_t>(faces, aPoly);
        break;
      }
      case tinyply::Type::UINT32: {
        HandleFaces<uint32_t>(faces, aPoly);
        break;
      }
      default: {
        return nullptr;
      }
    }

    return aPoly;
  }
  catch (std::exception & anException) {
    return nullptr;
  }
}

#else
opencascade::handle<Poly_Triangulation> RWPly::ReadFile(const Standard_CString theFile,
                                                        const opencascade::handle<Message_ProgressIndicator> &theProgress) {
  std::cerr << "OCCT have not been compiled with TinyPly support" << std::endl;
  return nullptr;
}
#endif
