#include "auxiliary_methods.h"

struct CelestialBodyTexture {
    string diffuse;
    string specular;
    string normal;
};

CelestialBodyTexture celestialBodyTextures[] = {
    { "sun.jpg", "sun_SPEC.png", "sun_NRM.png" },
    
    { "mercury.jpg", "mercury_SPEC.png", "mercury_NRM.png" },
    
    { "venus.jpg", "venus_SPEC.png", "venus_NRM.png" },
    
    { "earth.jpg", "earth_SPEC.png", "earth_NRM.png" },
    
    { "mars.jpg", "mars_SPEC.png", "mars_NRM.png" },
    
    { "jupiter.jpg", "jupiter_SPEC.png", "jupiter_NRM.png" },
    
    { "saturn.jpg", "saturn_SPEC.png", "saturn_NRM.png" },
    
    { "uranus.jpg", "uranus_SPEC.png", "uranus_NRM.png" },
    
    { "neptune.jpg", "neptune_SPEC.png", "neptune_NRM.png" },
    
    { "pluto.jpg", "pluto_SPEC.png", "pluto_NRM.png" }
};

struct PlanetProperty {
    float revolutionRate;
    float intialAngle;
    Cvec3 size;
    Cvec3 radius;
    Cvec3 orbit;
    CelestialBodyTexture images;
    TextureBinder texture;
    Entity *planetEntity;
};

PlanetProperty planetProperties[] = {
    { 01.00, 00.00, Cvec3(1.80, 1.80, 1.80), Cvec3(00.0, 0.0, 0.0), Cvec3(0.0, 0.0, 0.0), celestialBodyTextures[0] },
    
    { 47.89, -80.0, Cvec3(0.20, 0.20, 0.20), Cvec3(03.0, 0.0, 0.0), Cvec3(1.0/2.0, 1.0/2.0, 1.0/2.0), celestialBodyTextures[1] },
    
    { 35.03, -70.0, Cvec3(0.30, 0.30, 0.30), Cvec3(04.5, 0.0, 0.0), Cvec3(1.0/1.35, 1.0/1.35, 1.0/1.35), celestialBodyTextures[2] },
    
    { 29.79, 45.00, Cvec3(0.50, 0.50, 0.50), Cvec3(06.0, 0.0, 0.0), Cvec3(1.0, 1.0, 1.0), celestialBodyTextures[3] },
    
    { 24.13, -30.0, Cvec3(0.40, 0.40, 0.40), Cvec3(08.0, 0.0, 0.0), Cvec3(1.35, 1.0, 1.35), celestialBodyTextures[4] },
    
    { 13.06, -40.0, Cvec3(1.00, 1.00, 1.00), Cvec3(11.0, 0.0, 0.0), Cvec3(1.85, 1.0, 1.85), celestialBodyTextures[5] },
    
    { 09.64, -45.0, Cvec3(0.80, 0.80, 0.80), Cvec3(14.5, 0.0, 0.0), Cvec3(2.40, 1.0, 2.40), celestialBodyTextures[6] },
    
    { 06.81, 45.00, Cvec3(0.30, 0.30, 0.30), Cvec3(17.0, 0.0, 0.0), Cvec3(2.825, 1.0, 2.825), celestialBodyTextures[7] },
    
    { 05.43, 60.00, Cvec3(0.25, 0.25, 0.25), Cvec3(18.0, 0.0, 0.0), Cvec3(3.0, 1.0, 3.0), celestialBodyTextures[8] },
    
    { 04.00, 70.00, Cvec3(0.15, 0.15, 0.15), Cvec3(19.0, 0.0, 0.0), Cvec3(3.165, 1.0, 3.165), celestialBodyTextures[9] }
};

