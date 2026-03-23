#pragma once

#include "geometry/vertex.h"
#include "math/vector2f.h"
#include "math/vector3f.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using namespace sr::math;

namespace sr {
namespace geometry {

struct Mesh {
  std::vector<Vertex> vertices; // vertex data
  std::vector<int> indices;     // index data

  // load mesh data from .obj file
  // o [Name]
  // - means a new object
  // v [x] [y] [z]
  // - means a new vertex with position (x, y, z)
  // vt [u] [v]
  // - means a new vertex with texture coordinate (u, v)
  // vn [x] [y] [z]
  // - means a new vertex with normal (x, y, z)
  // f [v/vt/vn] [v/vt/vn] [v/vt/vn]
  // - means a new face with vertex indices (v/vt/vn)
  bool loadFromObj(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Failed to open OBJ file: " << filename << std::endl;
      return false;
    }

    std::vector<Vector3f> temp_positions;
    std::vector<Vector3f> temp_normals;
    std::vector<Vector2f> temp_texcoords;

    std::string line;
    while (std::getline(file, line)) {
      std::istringstream iss(line); // split the line by space
      std::string prefix;
      iss >> prefix;

      if (prefix == "v") { // vertex position
        float x, y, z;
        iss >> x >> y >> z;
        temp_positions.emplace_back(x, y, z);
      } else if (prefix == "vn") { // vertex normal
        float x, y, z;
        iss >> x >> y >> z;
        temp_normals.emplace_back(x, y, z);
      } else if (prefix == "vt") { // vertex texture coordinate
        float u, v;
        iss >> u >> v;
        temp_texcoords.emplace_back(u, v);
      } else if (prefix == "f") { // face
        std::vector<std::string> faceVerts;
        std::string vertStr;
        while (iss >> vertStr) {
          faceVerts.push_back(vertStr);
        } // get all vertex indices in the face

        if (faceVerts.size() < 3)
          continue; // ignore degenerate faces

        // triangulate the face
        // {0,1,2}, {0,2,3}, {0,3,4},...
        for (size_t i = 1; i < faceVerts.size() - 1; ++i) {
          int triIndices[3] = {0, static_cast<int>(i), static_cast<int>(i + 1)};

          for (int j = 0; j < 3; ++j) {
            int pos_idx = 0, tex_idx = 0, norm_idx = 0;
            parseVertexIndex(faceVerts[triIndices[j]], pos_idx, tex_idx,
                             norm_idx);
            pos_idx -= 1;
            tex_idx -= 1;
            norm_idx -= 1;

            // construct vertex
            Vertex v;
            v.position = (pos_idx >= 0 && pos_idx < temp_positions.size())
                             ? temp_positions[pos_idx]
                             : Vector3f(0, 0, 0);
            v.texcoord = (tex_idx >= 0 && tex_idx < temp_texcoords.size())
                             ? temp_texcoords[tex_idx]
                             : Vector2f(0, 0);
            v.normal = (norm_idx >= 0 && norm_idx < temp_normals.size())
                           ? temp_normals[norm_idx]
                           : Vector3f(0, 0, 1); // 默认朝前
            v.color = Vector3f(1, 1, 1);

            int currentIndex = vertices.size();
            vertices.push_back(v);
            indices.push_back(currentIndex);
          }
        }
      }
    }
    file.close();
    std::cout << "Loaded mesh: " << filename << " (" << vertices.size()
              << " vertices, " << indices.size() / 3 << " triangles)"
              << std::endl;
    return true;
  };

private:
  void parseVertexIndex(const std::string &str, int &pos, int &tex, int &norm) {
    pos = tex = norm = 0;

    size_t slash_pos = str.find('/');
    size_t slash_pos2 = str.find('/', slash_pos + 1);

    if (slash_pos == std::string::npos) // only position
      pos = std::stoi(str);
    else {
      pos = std::stoi(str.substr(0, slash_pos));
      if (slash_pos2 == std::string::npos) { // position and texture
        if (slash_pos + 1 < str.size())
          tex = std::stoi(str.substr(slash_pos + 1));
      } else { // position, texture, and normal
        if (slash_pos + 1 < slash_pos2)
          tex =
              std::stoi(str.substr(slash_pos + 1, slash_pos2 - slash_pos - 1));
        if (slash_pos2 + 1 < str.size())
          norm = std::stoi(str.substr(slash_pos2 + 1));
      }
    }
  }
};
} // namespace geometry
} // namespace sr