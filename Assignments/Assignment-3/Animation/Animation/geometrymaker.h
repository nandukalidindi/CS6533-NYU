#ifndef GEOMETRYMAKER_H
#define GEOMETRYMAKER_H

#include <cmath>
#include <vector>
#include "cvec.h"


using namespace std;
//--------------------------------------------------------------------------------
// Helpers for creating some special geometries such as plane, cubes, and spheres
//--------------------------------------------------------------------------------


// A generic vertex structure containing position, normal, and texture information
// Used by make* functions to pass vertex information to the caller
struct GenericVertex {
  Cvec3f pos;
  Cvec3f normal;
  Cvec2f tex;
  Cvec3f tangent, binormal;

  GenericVertex(
    float x, float y, float z,
    float nx, float ny, float nz,
    float tu, float tv,
    float tx, float ty, float tz,
    float bx, float by, float bz)
    : pos(x,y,z), normal(nx,ny,nz), tex(tu, tv), tangent(tx, ty, tz), binormal(bx, by, bz)
  {}
};

inline void getPlaneVbIbLen(int& vbLen, int& ibLen) {
  vbLen = 4;
  ibLen = 6;
}

template<typename VtxOutIter, typename IdxOutIter>
void makePlane(float size, VtxOutIter vtxIter, IdxOutIter idxIter) {
  float h = size / 2.0;
  *vtxIter = GenericVertex(    -h, 0, -h, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, -1);
  *(++vtxIter) = GenericVertex(-h, 0,  h, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, -1);
  *(++vtxIter) = GenericVertex( h, 0,  h, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, -1);
  *(++vtxIter) = GenericVertex( h, 0, -h, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, -1);
  *idxIter = 0;
  *(++idxIter) = 1;
  *(++idxIter) = 2;
  *(++idxIter) = 0;
  *(++idxIter) = 2;
  *(++idxIter) = 3;
}

inline void getCubeVbIbLen(int& vbLen, int& ibLen) {
  vbLen = 24;
  ibLen = 36;
}

template<typename VtxOutIter, typename IdxOutIter>
void makeCube(float size, VtxOutIter vtxIter, IdxOutIter idxIter) {
  float h = size / 2.0;
#define DEFV(x, y, z, nx, ny, nz, tu, tv) { \
    *vtxIter = GenericVertex(x h, y h, z h, \
                             nx, ny, nz, tu, tv, \
                             tan[0], tan[1], tan[2], \
                             bin[0], bin[1], bin[2]); \
    ++vtxIter; \
}
  Cvec3f tan(0, 1, 0), bin(0, 0, 1);
  DEFV(+, -, -, 1, 0, 0, 0, 0); // facing +X
  DEFV(+, +, -, 1, 0, 0, 1, 0);
  DEFV(+, +, +, 1, 0, 0, 1, 1);
  DEFV(+, -, +, 1, 0, 0, 0, 1);

  tan = Cvec3f(0, 0, 1);
  bin = Cvec3f(0, 1, 0);
  DEFV(-, -, -, -1, 0, 0, 0, 0); // facing -X
  DEFV(-, -, +, -1, 0, 0, 1, 0);
  DEFV(-, +, +, -1, 0, 0, 1, 1);
  DEFV(-, +, -, -1, 0, 0, 0, 1);

  tan = Cvec3f(0, 0, 1);
  bin = Cvec3f(1, 0, 0);
  DEFV(-, +, -, 0, 1, 0, 0, 0); // facing +Y
  DEFV(-, +, +, 0, 1, 0, 1, 0);
  DEFV(+, +, +, 0, 1, 0, 1, 1);
  DEFV(+, +, -, 0, 1, 0, 0, 1);

  tan = Cvec3f(1, 0, 0);
  bin = Cvec3f(0, 0, 1);
  DEFV(-, -, -, 0, -1, 0, 0, 0); // facing -Y
  DEFV(+, -, -, 0, -1, 0, 1, 0);
  DEFV(+, -, +, 0, -1, 0, 1, 1);
  DEFV(-, -, +, 0, -1, 0, 0, 1);

  tan = Cvec3f(1, 0, 0);
  bin = Cvec3f(0, 1, 0);
  DEFV(-, -, +, 0, 0, 1, 0, 0); // facing +Z
  DEFV(+, -, +, 0, 0, 1, 1, 0);
  DEFV(+, +, +, 0, 0, 1, 1, 1);
  DEFV(-, +, +, 0, 0, 1, 0, 1);

  tan = Cvec3f(0, 1, 0);
  bin = Cvec3f(1, 0, 0);
  DEFV(-, -, -, 0, 0, -1, 0, 0); // facing -Z
  DEFV(-, +, -, 0, 0, -1, 1, 0);
  DEFV(+, +, -, 0, 0, -1, 1, 1);
  DEFV(+, -, -, 0, 0, -1, 0, 1);
#undef DEFV

  for (int v = 0; v < 24; v +=4) {
    *idxIter = v;
    *++idxIter = v + 1;
    *++idxIter = v + 2;
    *++idxIter = v;
    *++idxIter = v + 2;
    *++idxIter = v + 3;
    ++idxIter;
  }
}

inline void getSphereVbIbLen(int slices, int stacks, int& vbLen, int& ibLen) {
  assert(slices > 1);
  assert(stacks >= 2);
  vbLen = (slices + 1) * (stacks + 1);
  ibLen = slices * stacks * 6;
}

//void calculateFaceTangent(const Cvec3f &v1, const Cvec3f &v2, const Cvec3f &v3, const Cvec2f &texCoord1, const Cvec2f &texCoord2, const Cvec2f &texCoord3, Cvec3f &tangent, Cvec3f &binormal) {
//    Cvec3f side0 = v1 - v2;
//    Cvec3f side1 = v3 - v1;
//    Cvec3f normal = cross(side1, side0);
//    if (normal[0] != 0 && normal[1] != 0 && normal[2] != 0)
//        normalize(normal);
//    float deltaV0 = texCoord1[1] - texCoord2[1];
//    float deltaV1 = texCoord3[1] - texCoord1[1];
//    tangent = side0 * deltaV1 - side1 * deltaV0;
//    if (tangent[0] != 0 && tangent[1] != 0 && tangent[2] != 0)
//        normalize(tangent);
//    float deltaU0 = texCoord1[0] - texCoord2[0];
//    float deltaU1 = texCoord3[0] - texCoord1[0];
//    binormal = side0 * deltaU1 - side1 * deltaU0;
//    if (binormal[0] != 0 && binormal[1] != 0 && binormal[2] != 0)
//        normalize(binormal);
//    Cvec3f tangentCross = cross(tangent, binormal);
//    if (dot(tangentCross, normal) < 0.0f) {
//        tangent = tangent * -1;
//    }
//}

inline void getTorus(int segmentsW, int segmentsH, int& vbLen, int& ibLen) {
    assert(segmentsW > 1);
    assert(segmentsH >= 2);
    vbLen = (segmentsW + 1) * (segmentsH + 1);
    ibLen = segmentsW * segmentsH * 6;
}

//template<typename VtxOutIter, typename IdxOutIter>
//void makeTorus(float radius, float tubeRadius, int segmentsW, int segmentsH, float tiltingValue, VtxOutIter vtxIter, IdxOutIter idxIter) {
//    vector<Cvec3f> coordinates;
//    vector<Cvec2f> texCoordinates;
//    vector<Cvec3f> normals;
//    vector<Cvec3f> tangents;
//    vector<Cvec3f> binormals;
//    segmentsW++;
//    segmentsH++;
//    
//    float tDelta = 360.f/(segmentsW-1);
//    float pDelta = 360.f/(segmentsH-1);
//    
//    float phi = -90;
//    float theta = 0;
//    
//    for(int i=0; i<segmentsH; i++) {
//        for(int j=0; j<segmentsW; j++) {
//            float x = (radius + tubeRadius*cos(phi*0.0174532925))*cos(theta*0.0174532925);
//            float y = tubeRadius*sin(phi*0.0174532925);
//            float z = (radius + tubeRadius*cos(phi*0.0174532925))*sin(theta*0.0174532925);
//            
//            coordinates.push_back(Cvec3f(x, y, z));
//            
//            float u = (-theta / (360.f))*tiltingValue;
//            float v = ((phi / (360.f)) + 0.5)*tiltingValue;
//            
//            texCoordinates.push_back(Cvec2f(u, v));
//            
//            theta += tDelta;
//            
//
//        }
//        phi += pDelta;
//        theta = 0;
//    }
//    
//    for (int i = 0; i < segmentsW + 1; ++i) {
//        for (int j = 0; j < segmentsH + 1; ++j) {
//            if (i < segmentsW && j < segmentsH ) {
//                *idxIter = (segmentsH+1) * i + j;
//                *++idxIter = (segmentsH+1) * i + j + 1;
//                *++idxIter = (segmentsH+1) * (i + 1) + j + 1;
//                
//                *++idxIter = (segmentsH+1) * i + j;
//                *++idxIter = (segmentsH+1) * (i + 1) + j + 1;
//                *++idxIter = (segmentsH+1) * (i + 1) + j;
//                ++idxIter;
//            }
//
//        }
//    }
//    
//    for(int i=0; i<coordinates.size(); i+=3) {
//        Cvec3f e1 = coordinates[i] - coordinates[i+1];
//        Cvec3f e2 = coordinates[i+2] - coordinates[i+1];
//        Cvec3f normal = (cross(e1, e2));
//        
//        normals.push_back(normal);
//        normals.push_back(normal);
//        normals.push_back(normal);
//    }
//    
//    for(int i=0; i<coordinates.size(); i += 3) {
//        Cvec3f tangent;
//        Cvec3f binormal;
//        calculateFaceTangent(coordinates[i], coordinates[i+1], coordinates[i+2], texCoordinates[i], texCoordinates[i+1], texCoordinates[i+2], tangent, binormal);
//        
//        tangents.push_back(tangent);
//        tangents.push_back(tangent);
//        tangents.push_back(tangent);
//        
//        binormals.push_back(binormal);
//        binormals.push_back(binormal);
//        binormals.push_back(binormal);
//    }
//    
//    for(int i=0; i<coordinates.size(); i++) {
//        *vtxIter = GenericVertex(
//                                 coordinates[i][0], coordinates[i][1], coordinates[i][2],
//                                 normals[i][0], normals[i][1], normals[i][2],
//                                 texCoordinates[i][0], texCoordinates[i][1],
//                                 tangents[i][0], tangents[i][1], tangents[i][2],
//                                 binormals[i][0], binormals[i][1], binormals[i][2]);
//        ++vtxIter;
//    }
//    
//    
//}

template<typename VtxOutIter, typename IdxOutIter>
void makeSphere(float radius, int slices, int stacks, VtxOutIter vtxIter, IdxOutIter idxIter) {
  using namespace std;
  assert(slices > 1);
  assert(stacks >= 2);

  const double radPerSlice = 2 * CS175_PI / slices;
  const double radPerStack = CS175_PI / stacks;

  vector<double> longSin(slices+1), longCos(slices+1);
  vector<double> latSin(stacks+1), latCos(stacks+1);
    
  for (int i = 0; i < slices + 1; ++i) {
    longSin[i] = sin(radPerSlice * i);
    longCos[i] = cos(radPerSlice * i);
  }
  for (int i = 0; i < stacks + 1; ++i) {
    latSin[i] = sin(radPerStack * i);
    latCos[i] = cos(radPerStack * i);
  }

  for (int i = 0; i < slices + 1; ++i) {
    for (int j = 0; j < stacks + 1; ++j) {
      float x = longCos[i] * latSin[j];
      float y = longSin[i] * latSin[j];
      float z = latCos[j];

      Cvec3f n(x, y, z);
      Cvec3f t(-longSin[i], longCos[i], 0);
      Cvec3f b = cross(n, t);

      *vtxIter = GenericVertex(
        x * radius, y * radius, z * radius,
        x, y, z,
        1.0/slices*i, 1.0/stacks*j,
        t[0], t[1], t[2],
        b[0], b[1], b[2]);
      ++vtxIter;

      if (i < slices && j < stacks ) {
        *idxIter = (stacks+1) * i + j;
        *++idxIter = (stacks+1) * i + j + 1;
        *++idxIter = (stacks+1) * (i + 1) + j + 1;

        *++idxIter = (stacks+1) * i + j;
        *++idxIter = (stacks+1) * (i + 1) + j + 1;
        *++idxIter = (stacks+1) * (i + 1) + j;
        ++idxIter;
      }
    }
  }
}


#endif
