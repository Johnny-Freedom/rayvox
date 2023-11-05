#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "raygui.h"

#define FNL_IMPL
#include "FastNoiseLite.h"

typedef unsigned long long UINT64;
typedef signed long long   SINT64;

typedef unsigned int       UINT32;
typedef signed int         SINT32;

typedef unsigned short     UINT16;
typedef signed short       SINT16;

typedef unsigned char      UINT8;
typedef signed char        SINT8;

typedef float              SFLOAT32;
typedef double             SFLOAT64;

typedef  char              SCHAR;
typedef unsigned char      UCHAR;


typedef bool BOOL;
#define FALSE false
#define TRUE  true

typedef SINT8 CONTINUITY;

bool IsLessVSINT16(const struct VSINT16* a1, const struct VSINT16* a2);

typedef struct VSINT16 {
    SINT16 x;
    SINT16 y;
    SINT16 z;

    //bool operator<(const VSINT16& rhs) const {
    //    return IsLessVSINT16(this, &rhs);
        // TEMP TEMP
        //return Vector3Distance({ 0,0,0 }, { (SFLOAT32)x, (SFLOAT32)y, (SFLOAT32)z }) < Vector3Distance({ 0,0,0 }, { (SFLOAT32)rhs.x, (SFLOAT32)rhs.y, (SFLOAT32)rhs.z });
    //}

} VSINT16;

bool IsLessVSINT16(const struct VSINT16* a1, const struct VSINT16* a2) {

    if (a1->x < a2->x) {
        return TRUE;
    }
    if (a1->y < a2->y) {
        return TRUE;
    }
    if (a1->z < a2->z) {
        return TRUE;
    }
    return FALSE;
}

#define EXTERN_C extern "C"


#define _CALLERALLOC_
#define _IN_
#define _OUT_



#include <vector>
#include <map>
#include <string>

RLAPI EXTERN_C Vector3 GetCameraForward(Camera* camera);

UINT16 g_widthWindow  = 800;
UINT16 g_heightWindow = 600;

Camera gFreeCamera = { 0 };

#define CHUNK_SIZE 32

typedef struct BLOCKENTRYDATA {
	//=============================================================================
	// Chunker Cares about these
	//=============================================================================
    UINT16 TypeId; 
	
	//=============================================================================
	// The Climate (and some blocks) care about these
	//=============================================================================
	UINT16 Temperature;
	UINT8 Saturation;
    UINT8 Light;
    UINT8 Damage;
    UINT8 Pressure;
	
	
} BLOCKENTRYDATA;
typedef struct CHUNK {
    BLOCKENTRYDATA BlockData[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    Mesh   ChunkMesh;
    Model  ChunkModel;

    VSINT16 position;
    CHUNK* neighborXP;
    CHUNK* neighborXN;
    CHUNK* neighborYP;
    CHUNK* neighborYN;
    CHUNK* neighborZP;
    CHUNK* neighborZN;

    BOOL  bChanged;

    BOOL  bLoaded;


    // CLIENT-SPECIFIC
    UINT8 Lod;
    BOOL  bRender;

    UINT16 countMeshings;

    UINT32 statIndices;
    UINT32 statVertices;

} CHUNK;

typedef struct WORLD {
    std::string Id;
    std::map< SINT16, std::map< SINT16, std::map<SINT16, CHUNK*>>> ChunkList;

} WORLD;

typedef UINT8 PACKID;

typedef struct BLOCKTYPE {
	PACKID   PackID;
	std::string NameID;
    Vector2  TexOrigin;
    Vector2  TexEnd;
    SFLOAT32 sDensity;
    SFLOAT32 lDensity;
    SFLOAT32 gDensity;
    SFLOAT32 MeltingPoint;
    SFLOAT32 BoilingPoint;
} BLOCKTYPE;

std::vector<BLOCKTYPE> BlockTypes;

typedef struct PACKDATA {
	std::map<std::string, BLOCKTYPE*> BlockTypes;
} PACKDATA;

std::vector<PACKDATA> Packs;

CONTINUITY worldAddInChunk(_IN_ _OUT_ WORLD* pWorld, VSINT16 Coordinate, _IN_ _OUT_ CHUNK* pChunk
    // -0x101 == A chunk is already existant at position

) {
    if (pWorld == NULL) {
        return -0x10;
    }
    if (pChunk == NULL) {
        return -0x11;
    }
    if (pWorld->ChunkList.count(Coordinate.x) > 0 && pWorld->ChunkList[Coordinate.x].count(Coordinate.y) > 0 && pWorld->ChunkList[Coordinate.x][Coordinate.y].count(Coordinate.z) > 0) {
        return -0x101;
    }

    pChunk->position.x = Coordinate.x;
    pChunk->position.y = Coordinate.y;
    pChunk->position.z = Coordinate.z;


    pWorld->ChunkList[Coordinate.x][Coordinate.y][Coordinate.z] = pChunk;
    CHUNK* pOtherChunk;


    VSINT16 position = { Coordinate.x + 0, Coordinate.y + 0, Coordinate.z + 0 };

    position.x = Coordinate.x + 1;
    position.y = Coordinate.y + 0;
    position.z = Coordinate.z + 0;
    if (pWorld->ChunkList.count(position.x) > 0 && pWorld->ChunkList[position.x].count(position.y) > 0 && pWorld->ChunkList[position.x][position.y].count(position.z) > 0) {
        pWorld->ChunkList[position.x][position.y][position.z]->neighborXN = pChunk;
        pChunk->neighborXP = pWorld->ChunkList[position.x][position.y][position.z];
    }
    position.x = Coordinate.x - 1;
    position.y = Coordinate.y + 0;
    position.z = Coordinate.z + 0;
    if (pWorld->ChunkList.count(position.x) > 0 && pWorld->ChunkList[position.x].count(position.y) > 0 && pWorld->ChunkList[position.x][position.y].count(position.z) > 0) {
        pWorld->ChunkList[position.x][position.y][position.z]->neighborXP = pChunk;
        pChunk->neighborXN = pWorld->ChunkList[position.x][position.y][position.z];
    }
    position.x = Coordinate.x + 0;
    position.y = Coordinate.y + 1;
    position.z = Coordinate.z + 0;
    if (pWorld->ChunkList.count(position.x) > 0 && pWorld->ChunkList[position.x].count(position.y) > 0 && pWorld->ChunkList[position.x][position.y].count(position.z) > 0) {
        pWorld->ChunkList[position.x][position.y][position.z]->neighborYN = pChunk;
        pChunk->neighborYP = pWorld->ChunkList[position.x][position.y][position.z];
    }
    position.x = Coordinate.x + 0;
    position.y = Coordinate.y - 1;
    position.z = Coordinate.z + 0;
    if (pWorld->ChunkList.count(position.x) > 0 && pWorld->ChunkList[position.x].count(position.y) > 0 && pWorld->ChunkList[position.x][position.y].count(position.z) > 0) {
        pWorld->ChunkList[position.x][position.y][position.z]->neighborYP = pChunk;
        pChunk->neighborYN = pWorld->ChunkList[position.x][position.y][position.z];
    }
    position.x = Coordinate.x + 0;
    position.y = Coordinate.y + 0;
    position.z = Coordinate.z + 1;
    if (pWorld->ChunkList.count(position.x) > 0 && pWorld->ChunkList[position.x].count(position.y) > 0 && pWorld->ChunkList[position.x][position.y].count(position.z) > 0) {
        pWorld->ChunkList[position.x][position.y][position.z]->neighborZN = pChunk;
        pChunk->neighborZP = pWorld->ChunkList[position.x][position.y][position.z];
    }
    position.x = Coordinate.x + 0;
    position.y = Coordinate.y + 0;
    position.z = Coordinate.z - 1;
    if (pWorld->ChunkList.count(position.x) > 0 && pWorld->ChunkList[position.x].count(position.y) > 0 && pWorld->ChunkList[position.x][position.y].count(position.z) > 0) {
        pWorld->ChunkList[position.x][position.y][position.z]->neighborZP = pChunk;
        pChunk->neighborZN = pWorld->ChunkList[position.x][position.y][position.z];
    }

    return 1;
}

CONTINUITY worldUnloadChunk() {

    return 0;
}
CONTINUITY worldProcess(_IN_ _OUT_ WORLD* pWorld) {
    if (pWorld == NULL) {
        return -0x10;
    }

    return 1;
}
CONTINUITY worldRender(_IN_ WORLD* pWorld) {
    if (pWorld == NULL) {
        return -0x10;
    }

    //https://web.archive.org/web/20160327184424/http://ruh.li/CameraViewFrustum.html

    for (auto it0 = pWorld->ChunkList.begin(); it0 != pWorld->ChunkList.end(); ++it0) {
        for (auto it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
            for (auto it = it1->second.begin(); it != it1->second.end(); ++it) {
                if (it->second->bRender == TRUE) {
                    DrawModel(it->second->ChunkModel, { (SFLOAT32)it->second->position.x * CHUNK_SIZE,  (SFLOAT32)it->second->position.y * CHUNK_SIZE,  (SFLOAT32)it->second->position.z * CHUNK_SIZE }, 1.f, WHITE);
                }
            }
        }
    }

    return 1;
}

SFLOAT32 g_terrain_ScaleX = .5;
SFLOAT32 g_terrain_ScaleY = 15;
SFLOAT32 g_terrain_ScaleZ = .5;

CHUNK* worldLoadOrCreateChunk(_IN_ WORLD* pWorld, VSINT16 coordinate) {
    CHUNK* NewChunk = NULL;
    if (pWorld->ChunkList.count(coordinate.x) > 0 && pWorld->ChunkList[coordinate.x].count(coordinate.y) > 0 && pWorld->ChunkList[coordinate.x][coordinate.y].count(coordinate.z) > 0) {
        if (pWorld->ChunkList[coordinate.x][coordinate.y][coordinate.z]->bLoaded == FALSE ) {
            NewChunk = pWorld->ChunkList[coordinate.x][coordinate.y][coordinate.z];
            goto GENERATE;
        }
        return pWorld->ChunkList[coordinate.x][coordinate.y][coordinate.z];
    }
    // Attempt load from Disk
    
    // Create Chunk
    NewChunk = (CHUNK*)MemAlloc(sizeof(CHUNK));
    NewChunk->bRender = TRUE;
    NewChunk->bChanged = TRUE;
    CONTINUITY Continuity = worldAddInChunk(pWorld, coordinate, NewChunk);
    if (Continuity < 0) {
        __debugbreak();
    }
GENERATE: 
    if (NewChunk == NULL) {
        __debugbreak();
    }

    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;

    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {
            //for (int iY = 0; iY < CHUNK_SIZE; iY++) {
            float NoiseVal = fnlGetNoise2D(&noise, (NewChunk->position.x * CHUNK_SIZE + iX) * g_terrain_ScaleX, (NewChunk->position.z * CHUNK_SIZE + iZ) * g_terrain_ScaleZ) * g_terrain_ScaleY;
            NoiseVal *= 0.4 * (fnlGetNoise2D(&noise, (NewChunk->position.x * CHUNK_SIZE + iX) * g_terrain_ScaleX / 2, (NewChunk->position.z * CHUNK_SIZE + iZ) * g_terrain_ScaleZ / 2) * g_terrain_ScaleY / 2);
            //NoiseVal *= 0.01 * (fnlGetNoise2D(&noise, (NewChunk->position.x * CHUNK_SIZE + iX) * g_terrain_ScaleX *10, (NewChunk->position.z * CHUNK_SIZE + iZ) * g_terrain_ScaleZ * 10) * g_terrain_ScaleY / 2);
            for (int iY = 0; iY < CHUNK_SIZE; iY++) {
                //float NoiseVal = fnlGetNoise3D(&noise, NewChunk->position.x*CHUNK_SIZE + iX, NewChunk->position.y * CHUNK_SIZE + iY, NewChunk->position.z * CHUNK_SIZE + iZ) * 12 / (iY * NewChunk->position.y * CHUNK_SIZE);
                //if (NoiseVal > 0) {
                if ((NewChunk->position.y * CHUNK_SIZE + iY) < NoiseVal ) {
                    NewChunk->BlockData[iX][iY][iZ].TypeId = 2;
                } else if ((NewChunk->position.y * CHUNK_SIZE + iY) < 0) {
                    NewChunk->BlockData[iX][iY][iZ].TypeId = 3;
                }
            }
        }
    }

    NewChunk->bLoaded = TRUE;
}

std::vector<CHUNK*> ChunkLoadList;

CHUNK* worldMarkChunkToBeLoaded(_IN_ WORLD* pWorld, VSINT16 coordinate) {

    CHUNK* NewChunk = (CHUNK*)MemAlloc(sizeof(CHUNK));
    //NewChunk->bRender = TRUE;
    NewChunk->bChanged = TRUE;
    CONTINUITY Continuity = worldAddInChunk(pWorld, coordinate, NewChunk);
    if (Continuity < 0) {
        __debugbreak();
    }

    for (int i = 0; i < ChunkLoadList.size(); i++) {
        if (ChunkLoadList[i] == NULL) {
            ChunkLoadList[i] = NewChunk;
            return NewChunk;
        }
    }
    ChunkLoadList.push_back(NewChunk);

    NewChunk->bLoaded = FALSE;
}

void worldLoadSomeChunks(_IN_ WORLD* pWorld, SINT16 originX, SINT16 originY, SINT16 originZ) {

    for (int i = 0; i < ChunkLoadList.size(); i++) {
        if (ChunkLoadList[i] != NULL) {
            ChunkLoadList[i]->bRender = TRUE;
            worldLoadOrCreateChunk(pWorld, ChunkLoadList[i]->position);
            ChunkLoadList[i] = NULL;
            return;
        }
    }
}

CONTINUITY worldLoadChunksPerCoords(_IN_ WORLD* pWorld, SINT16 originX, SINT16 originY, SINT16 originZ, SINT16 distanceX, SINT16 distanceY, SINT16 distanceZ) {
    if (pWorld == NULL) {
        return -0x10;
    }
    VSINT16 position = { originX + 0, originY + 0, originZ + 0 };

    CHUNK* pOrigin;
    if (pWorld->ChunkList.count(position.x) > 0 && pWorld->ChunkList[position.x].count(position.y) > 0 && pWorld->ChunkList[position.x][position.y].count(position.z) > 0) {
        pOrigin = pWorld->ChunkList[position.x][position.y][position.z];
    } else {
        pOrigin = worldMarkChunkToBeLoaded(pWorld, position);
    }
    for (SINT16 r = 0; r < distanceX; r++) {
        for (SINT16 i = -r + 1; i <= r; i++) {
            for (SINT16 j = -distanceY; j <= distanceY; j++) {
                if (!(pWorld->ChunkList.count(i) > 0 && pWorld->ChunkList[i].count(j) > 0 && pWorld->ChunkList[i][j].count(r) > 0)) {
                    worldMarkChunkToBeLoaded(pWorld, { i,j,r });
                
                }
            }
        }
        for (SINT16 i = -r ; i <= r; i++) {
            for (SINT16 j = -distanceY; j <= distanceY; j++) {
                if (!(pWorld->ChunkList.count(r) > 0 && pWorld->ChunkList[r].count(j) > 0 && pWorld->ChunkList[r][j].count(i) > 0)) {
                    worldMarkChunkToBeLoaded(pWorld, { r,j,i });
                }
            }
        }
        for (SINT16 i = -r + 1; i <= r; i++) {
            for (SINT16 j = -distanceY; j <= distanceY; j++) {
                if (!(pWorld->ChunkList.count(i) > 0 && pWorld->ChunkList[i].count(j) > 0 && pWorld->ChunkList[i][j].count(-r) > 0)) {
                    worldMarkChunkToBeLoaded(pWorld, { i,j,-r });
                }
            }
        }
        for (SINT16 i = -r; i <= r; i++) {
            for (SINT16 j = -distanceY; j <= distanceY; j++) {
                if (!(pWorld->ChunkList.count(-r) > 0 && pWorld->ChunkList[-r].count(j) > 0 && pWorld->ChunkList[-r][j].count(i) > 0)) {
                    worldMarkChunkToBeLoaded(pWorld, { -r,j,i });
                }
            }
        }
    }

}

CONTINUITY MeshChunk(_IN_ CHUNK* pChunk, _OUT_ _CALLERALLOC_ Mesh* pMesh) {

    if (pChunk == NULL) {
        return -0x10;
    }
    if (pMesh == NULL) {
        return -0x11;
    }


    int Faces = 0;
    //std::vector<Vector3> Vertices;
    //std::vector<UINT32>  Indices;
    //std::vector<Vector2> TexCoords;

    UINT32 numVertices  = 0;
    UINT32 numIndices   = 0;
    UINT32 numTexCoords = 0;

    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iY = 0; iY < CHUNK_SIZE; iY++) {
            for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {

                if (pChunk->BlockData[iX][iY][iZ].TypeId == 0) {
                    continue;
                }

                BOOL visXN = iX != 0 ? pChunk->BlockData[iX - 1][iY][iZ].TypeId : pChunk->neighborXN != NULL ? pChunk->neighborXN->BlockData[CHUNK_SIZE - 1][iY][iZ].TypeId : 0;
                BOOL visXP = iX < CHUNK_SIZE - 1 ? pChunk->BlockData[iX + 1][iY][iZ].TypeId : pChunk->neighborXP != NULL ? pChunk->neighborXP->BlockData[0][iY][iZ].TypeId : 0;
                BOOL visYN = iY != 0 ? pChunk->BlockData[iX][iY - 1][iZ].TypeId : pChunk->neighborYN != NULL ? pChunk->neighborYN->BlockData[iX][CHUNK_SIZE - 1][iZ].TypeId : 0;
                BOOL visYP = iY < CHUNK_SIZE - 1 ? pChunk->BlockData[iX][iY + 1][iZ].TypeId : pChunk->neighborYP != NULL ? pChunk->neighborYP->BlockData[iX][0][iZ].TypeId : 0;
                BOOL visZN = iZ != 0 ? pChunk->BlockData[iX][iY][iZ - 1].TypeId : pChunk->neighborZN != NULL ? pChunk->neighborZN->BlockData[iX][iY][CHUNK_SIZE - 1].TypeId : 0;
                BOOL visZP = iZ < CHUNK_SIZE - 1 ? pChunk->BlockData[iX][iY][iZ + 1].TypeId : pChunk->neighborZP != NULL ? pChunk->neighborZP->BlockData[iX][iY][0].TypeId : 0;

                if (visZN == 0) {
                    numIndices += 6;
                    numTexCoords += 4;
                    numVertices += 4;
                }
                if (visZP == 0) {
                    numIndices += 6;
                    numTexCoords += 4;
                    numVertices += 4;
                }
                if (visXP == 0) {
                    numIndices += 6;
                    numTexCoords += 4;
                    numVertices += 4;
                }
                if (visXN == 0) {
                    numIndices += 6;
                    numTexCoords += 4;
                    numVertices += 4;
                }
                if (visYN == 0) {
                    numIndices += 6;
                    numTexCoords += 4;
                    numVertices += 4;
                }
                if (visYP == 0) {
                    numIndices += 6;
                    numTexCoords += 4;
                    numVertices += 4;

                }

            }
        }
    }
    UINT32 countVertices = numVertices;
    UINT32 countIndices = numIndices;
    UINT32 countTexCoords = numTexCoords;

    pChunk->statIndices = countIndices;
    pChunk->statVertices = countVertices;

    numVertices = 0;
    numIndices = 0;
    numTexCoords = 0;

    //pMesh->vertices      = (SFLOAT32*)MemAlloc(pMesh->vertexCount * sizeof(Vector3));
    SFLOAT32* vVertices  = (SFLOAT32*)MemAlloc(countVertices    * sizeof(Vector3));
    UINT16*   vIndices   = (UINT16*)  MemAlloc(countIndices * 3 * sizeof(UINT16));
    SFLOAT32* vTexCoords = (SFLOAT32*)MemAlloc(countTexCoords   * sizeof(Vector2));
    //meshChunk.normals   = MemAlloc(meshChunk.vertexCount * sizeof(Vector3));
    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iY = 0; iY < CHUNK_SIZE; iY++) {
            for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {

                if (pChunk->BlockData[iX][iY][iZ].TypeId == 0) {
                    continue;
                }

                BOOL visXN = iX != 0             ? pChunk->BlockData[iX - 1][iY][iZ].TypeId : pChunk->neighborXN != NULL ? pChunk->neighborXN->BlockData[CHUNK_SIZE - 1][iY][iZ].TypeId : 0;
                BOOL visXP = iX < CHUNK_SIZE - 1 ? pChunk->BlockData[iX + 1][iY][iZ].TypeId : pChunk->neighborXP != NULL ? pChunk->neighborXP->BlockData[0]             [iY][iZ].TypeId : 0;
                BOOL visYN = iY != 0             ? pChunk->BlockData[iX][iY - 1][iZ].TypeId : pChunk->neighborYN != NULL ? pChunk->neighborYN->BlockData[iX][CHUNK_SIZE - 1][iZ].TypeId : 0;
                BOOL visYP = iY < CHUNK_SIZE - 1 ? pChunk->BlockData[iX][iY + 1][iZ].TypeId : pChunk->neighborYP != NULL ? pChunk->neighborYP->BlockData[iX][0]             [iZ].TypeId : 0;
                BOOL visZN = iZ != 0             ? pChunk->BlockData[iX][iY][iZ - 1].TypeId : pChunk->neighborZN != NULL ? pChunk->neighborZN->BlockData[iX][iY][CHUNK_SIZE - 1].TypeId : 0;
                BOOL visZP = iZ < CHUNK_SIZE - 1 ? pChunk->BlockData[iX][iY][iZ + 1].TypeId : pChunk->neighborZP != NULL ? pChunk->neighborZP->BlockData[iX][iY][0             ].TypeId : 0;

                if (visZN == 0) {
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;

                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;


                    vIndices[numIndices + 0] = Faces + 0;
                    vIndices[numIndices + 1] = Faces + 1;
                    vIndices[numIndices + 2] = Faces + 2;
                    vIndices[numIndices + 3] = Faces + 1;
                    vIndices[numIndices + 4] = Faces + 3;
                    vIndices[numIndices + 5] = Faces + 2;
                    numIndices += 6;

                    Faces += 4;
                }
                if (visZP == 0) {
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;

                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;

                    vIndices[numIndices + 0] = Faces + 0;
                    vIndices[numIndices + 1] = Faces + 1;
                    vIndices[numIndices + 2] = Faces + 2;
                    vIndices[numIndices + 3] = Faces + 1;
                    vIndices[numIndices + 4] = Faces + 3;
                    vIndices[numIndices + 5] = Faces + 2;
                    numIndices += 6;

                    Faces += 4;
                }
                if (visXP == 0) {
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;

                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;

                    vIndices[numIndices + 0] = Faces + 0;
                    vIndices[numIndices + 1] = Faces + 1;
                    vIndices[numIndices + 2] = Faces + 2;
                    vIndices[numIndices + 3] = Faces + 1;
                    vIndices[numIndices + 4] = Faces + 3;
                    vIndices[numIndices + 5] = Faces + 2;
                    numIndices += 6;

                    Faces += 4;
                }
                if (visXN == 0) {
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;

                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;

                    vIndices[numIndices + 0] = Faces + 0;
                    vIndices[numIndices + 1] = Faces + 1;
                    vIndices[numIndices + 2] = Faces + 2;
                    vIndices[numIndices + 3] = Faces + 1;
                    vIndices[numIndices + 4] = Faces + 3;
                    vIndices[numIndices + 5] = Faces + 2;
                    numIndices += 6;

                    Faces += 4;
                }
                if (visYN == 0) {
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 0;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;

                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;

                    vIndices[numIndices + 0] = Faces + 0;
                    vIndices[numIndices + 1] = Faces + 1;
                    vIndices[numIndices + 2] = Faces + 2;
                    vIndices[numIndices + 3] = Faces + 1;
                    vIndices[numIndices + 4] = Faces + 3;
                    vIndices[numIndices + 5] = Faces + 2;
                    numIndices += 6;

                    Faces += 4;
                }
                if (visYP == 0) {
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 0;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 0;
                    numVertices += 3;
                    vVertices[numVertices + 0] = (SFLOAT32)iX + 1;
                    vVertices[numVertices + 1] = (SFLOAT32)iY + 1;
                    vVertices[numVertices + 2] = (SFLOAT32)iZ + 1;
                    numVertices += 3;

                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexEnd.y;
                    numTexCoords += 2;
                    vTexCoords[numTexCoords + 0] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.x;
                    vTexCoords[numTexCoords + 1] = BlockTypes[pChunk->BlockData[iX][iY][iZ].TypeId].TexOrigin.y;
                    numTexCoords += 2;

                    vIndices[numIndices + 0] = Faces + 0;
                    vIndices[numIndices + 1] = Faces + 1;
                    vIndices[numIndices + 2] = Faces + 2;
                    vIndices[numIndices + 3] = Faces + 1;
                    vIndices[numIndices + 4] = Faces + 3;
                    vIndices[numIndices + 5] = Faces + 2;
                    numIndices += 6;

                    Faces += 4;
                }

            }
        }
    }
    /*
    for (int i = 0; i < countVerticies; i++) {
        vVerticies[i * 3 + 0] = Vertices[i].x;
        vVerticies[i * 3 + 1] = Vertices[i].y;
        vVerticies[i * 3 + 2] = Vertices[i].z;
    }
    for (int i = 0; i < countIndices; i++) {
        vIndices[i] = Indices[i];
    }
    for (int i = 0; i < countTexCoords; i++) {
        vTexCoords[i * 2 + 0] = TexCoords[i].x;
        vTexCoords[i * 2 + 1] = TexCoords[i].y;
    }*/
    if (pChunk->countMeshings == 0) {
        pMesh->vertices  = vVertices;
        pMesh->indices   = vIndices;
        pMesh->texcoords = vTexCoords;

        pMesh->triangleCount = countIndices / 3;
        pMesh->vertexCount   = countVertices;
    } else {
#if 1
        UnloadMesh(*pMesh);
        memset(pMesh, 0, sizeof(Mesh));

        pMesh->triangleCount = countIndices / 3;
        pMesh->vertexCount = countVertices;
        pMesh->vertices = vVertices;
        pMesh->indices = vIndices;
        pMesh->texcoords = vTexCoords;
        UploadMesh(pMesh, FALSE);
        MemFree(pMesh->vertices);
        MemFree(pMesh->indices);
        MemFree(pMesh->texcoords);
#else

        // a2 is ShaderLocationIndex
        UpdateMeshBuffer(*pMesh, 0, vVertices,  countVertices  *     sizeof(Vector3), 0);
        UpdateMeshBuffer(*pMesh, 6, vIndices,   countIndices   * 3 * sizeof(UINT16),  0);
        UpdateMeshBuffer(*pMesh, 1, vTexCoords, countTexCoords *     sizeof(Vector2), 0); // WORKS
        
        pMesh->triangleCount = countIndices / 3;
        pMesh->vertexCount = countVertices;
        pMesh->vertices = vVertices;
        pMesh->indices = vIndices;
        pMesh->texcoords = vTexCoords;
#endif

        //MemFree(pMesh->vertices);
        //MemFree(pMesh->indices);
        //MemFree(pMesh->texcoords);
        

    }

    pChunk->bChanged = FALSE;

    return 1;
}

typedef struct PLAYER {
    CHUNK* pCurrentChunk;
} PLAYER;
typedef struct CLIENT {
    //
    // Settings
    //
    Vector3 RenderDistance;
} CLIENT;

typedef struct PHYSICALOBJECT {

    Vector3 Velocity;
    Vector3 Position;

} PHYSICALOBJECT;


// Given chunk or neighbor chunk
BLOCKENTRYDATA* worldGetBlockDataNearSafe(_IN_ WORLD* pWorld, _IN_ CHUNK* pChunk, SINT8 x, SINT8 y, SINT8 z) {
    if (pWorld == NULL) {
        return NULL;
    }
    if (pChunk == NULL) {
        return NULL;
    }

    CHUNK* _Chunk = pChunk;
    
    if (x >= CHUNK_SIZE) {
        _Chunk = pChunk->neighborXP;
        x = x % CHUNK_SIZE;
    } else if (x < 0) {
        _Chunk = pChunk->neighborXN;
        x = CHUNK_SIZE + x;
    }
    if (_Chunk == NULL) {
        return NULL;
    }
    if (y >= CHUNK_SIZE) {
        _Chunk = pChunk->neighborYP;
        y = y % CHUNK_SIZE;
    } else if (y < 0) {
        _Chunk = pChunk->neighborYN;
        y = CHUNK_SIZE + y;
    }
    if (_Chunk == NULL) {
        return NULL;
    }
    if (z >= CHUNK_SIZE) {
        _Chunk = pChunk->neighborZP;
        z = z % CHUNK_SIZE;
    } else if (z < 0) {
        _Chunk = pChunk->neighborZN;
        z = CHUNK_SIZE + z;
    }
    if (_Chunk == NULL) {
        return NULL;
    }

    return &_Chunk->BlockData[x][y][z];

}

Image BlocktextureAtlas = { 0 };

SINT32 blockatlasFirstAvailableX = 0;
SINT32 blockatlasFirstAvailableY = 0;

void blockatlasAdd(Image* pImage, Vector2* pOrigin, Vector2* pEnd) {
    Color kDefaultcolor = { 255, 255, 255, 0 };
    int i = 0;
    int j = 0;
    BOOL bFound = FALSE;
    for (i = 0; i < 2048 && !bFound; i++) {
        for (j = 0; j < 2048 && !bFound; j++) {

            Color colour = GetImageColor(BlocktextureAtlas, i, j);
            if (colour.r == 0 && colour.g == 0 && colour.b == 0 && colour.a == 255) {

                bFound = TRUE;
                for (int ii = 0; ii < pImage->width && i + ii < 2048; ii++) {
                    for (int jj = 0; jj < pImage->height && j + jj < 2048; jj++) {
                        Color colour = GetImageColor(BlocktextureAtlas, i + ii, j + jj);
                        if (colour.r != 0 || colour.g != 0 || colour.b != 0 || colour.a != 255) {
                            bFound = FALSE;
                            goto CONTINUESEARCH;
                        }
                    }
                }
                goto FINISHEDSEARCH;
            }
        CONTINUESEARCH:
            ;
        }
    }
    FINISHEDSEARCH:

    if (i >= 2048 - 1 && j >= 2048 - 1) {
        return;
    }

    pOrigin->x = (SFLOAT32)i/2048;
    pOrigin->y = (SFLOAT32)j / 2048;
    pEnd->x = (SFLOAT32)(i+pImage->width) / (SFLOAT32)2048;
    pEnd->y = (SFLOAT32)(j+pImage->height) / (SFLOAT32)2048;

    ImageDraw(&BlocktextureAtlas, *pImage, { 0,0,(SFLOAT32)pImage->width, (SFLOAT32)pImage->height }, { (SFLOAT32)i, (SFLOAT32)j, (SFLOAT32)pImage->width, (SFLOAT32)pImage->height }, { 255,255,255,255 });
}

Vector3 objectGetPredictedFuturePosition_Normalized(PHYSICALOBJECT* pObject, SFLOAT32 Step) {
    Vector3 NewPosition = Vector3Add(pObject->Position, pObject->Velocity);

    return  Vector3Add(pObject->Position, Vector3Scale(Vector3Normalize(pObject->Velocity), Step));
}
Vector3 objectGetPredictedFuturePosition(PHYSICALOBJECT* pObject, SFLOAT32 Step) {
    return Vector3Add(pObject->Position, Vector3Scale(pObject->Velocity, Step));
}

PACKID gDefaultPackId = 0;

PACKID RegisterPack(std::string NameID) {
    PACKDATA blank;
	Packs.push_back(blank);
	return Packs.size()-1;
}

void RegisterBlock(PACKID PackID, std::string NameID, BLOCKTYPE* pBlockType) {
	pBlockType->PackID = PackID;
    pBlockType->NameID = NameID;
	BlockTypes.push_back(*pBlockType);
	
	Packs[PackID].BlockTypes[NameID] = &BlockTypes[BlockTypes.size()-1];
}

SINT32 main(SINT32 argc, SCHAR* argv[]) {
    CONTINUITY Continuity = 1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(g_widthWindow, g_heightWindow, "raylib [core] example - basic window");

	// Register the default pack
	gDefaultPackId = RegisterPack("game");


    BlocktextureAtlas = GenImageColor(2048, 2048, {0,0,0,255});

    BLOCKTYPE blockAir = { 0 };
    blockAir.TexOrigin = {0,0};
    blockAir.TexEnd = {2048,2048 };
	RegisterBlock(gDefaultPackId, "air", &blockAir);

    BLOCKTYPE block404 = { 0 };
    Image image404 = LoadImage("res/textures/blockfaces/404.png");
    blockatlasAdd(&image404, &(block404.TexOrigin), &(block404.TexEnd));
    UnloadImage(image404);                                    // Unload image data from CPU memory (RAM)
	RegisterBlock(gDefaultPackId, "_debug_404", &block404);
	
    BLOCKTYPE blockColourBlock = { 0 };
    Image imageColourBlock = LoadImage("res/textures/blockfaces/pure_white.png");
    blockatlasAdd(&imageColourBlock, &(blockColourBlock.TexOrigin), &(blockColourBlock.TexEnd));
    UnloadImage(imageColourBlock);                                    // Unload image data from CPU memory (RAM)
	RegisterBlock(gDefaultPackId, "_debug_colourblock", &blockColourBlock);

    BLOCKTYPE blockDirt = { 0 };
    Image imageDirt = LoadImage("res/textures/blockfaces/dirt.png");
    blockatlasAdd(&imageDirt, &(blockDirt.TexOrigin), &(blockDirt.TexEnd));
    UnloadImage(imageDirt);                                    // Unload image data from CPU memory (RAM)
    blockDirt.sDensity = 81.156349;
    blockDirt.MeltingPoint = 65000;
	RegisterBlock(gDefaultPackId, "dirt", &blockDirt);

    BLOCKTYPE blockWater = { 0 };
    Image imageWater = LoadImage("res/textures/blockfaces/water.png");
    blockatlasAdd(&imageWater, &(blockWater.TexOrigin), &(blockWater.TexEnd));
    UnloadImage(imageWater);                                    // Unload image data from CPU memory (RAM)
    blockWater.sDensity = 57.233954;
    blockWater.MeltingPoint = 32;
    blockWater.lDensity = 62.4;
    blockWater.BoilingPoint = 212; 
    blockWater.gDensity = 36.832497;
	RegisterBlock(gDefaultPackId, "water", &blockWater);

    Image image = LoadImage("../../../res/textures/blockfaces/atlastest.png");  // Load image data into CPU memory (RAM)
    //Texture2D texture = LoadTextureFromImage(image);       // Image converted to texture, GPU memory (RAM -> VRAM)
    UnloadImage(image);                                    // Unload image data from CPU memory (RAM)

    //image = LoadImageFromTexture(texture);                 // Load image from GPU texture (VRAM -> RAM)
    //UnloadTexture(texture);                                // Unload texture from GPU memory (VRAM)

    //texture = LoadTextureFromImage(image);                 // Recreate texture from retrieved image data (RAM -> VRAM)
    //UnloadImage(image);                                    // Unload retrieved image data from CPU memory (RAM)
    Texture2D texture = LoadTextureFromImage(BlocktextureAtlas);

    DisableCursor();
    // SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    CLIENT Client = { 0 };
    Client.RenderDistance.x = 4;
    Client.RenderDistance.y = 4;
    Client.RenderDistance.z = 4;
    PLAYER Player = { 0 };

    WORLD World;
	/*
    CHUNK Chunk = { 0 };
    CHUNK Chunk2 = { 0 };

    Player.pCurrentChunk = &Chunk;

    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iY = 0; iY < 2; iY++) {
            for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {

                Chunk.BlockData[iX][iY][iZ].TypeId = 1;
            }
        }
    }
    for (int iX = 0; iX < 2; iX++) {
        for (int iY = 2; iY < 4; iY++) {
            for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {

                Chunk.BlockData[iX][iY][iZ].TypeId = 1;
            }
        }
    }
    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iY = 16; iY < 25; iY++) {
            for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {

                Chunk.BlockData[iX][iY][iZ].TypeId = 1;
            }
        }
    }
    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iY = 0; iY < CHUNK_SIZE; iY++) {
            for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {

                Chunk2.BlockData[iX][iY][iZ].TypeId = 1;
            }
        }
    }
    for (int iX = 0; iX < 4; iX++) {
        for (int iY = 2; iY < 5; iY++) {
            for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {

                Chunk2.BlockData[iX][iY][iZ].TypeId = 1;
            }
        }
    }

    Continuity = worldAddInChunk(&World, { 0,0,0 }, &Chunk);
    if (Continuity < 1) {
        __debugbreak();
    }
   Continuity = worldAddInChunk(&World, { 0,0,1 }, &Chunk2);
    if (Continuity < 1) {
        __debugbreak();
    }

    Chunk.bChanged = TRUE;
    Chunk.bRender = TRUE;
    Chunk2.bChanged = TRUE;
    Chunk2.bRender = TRUE;
	*/

    //Continuity = MeshChunk(&Chunk, &Chunk.ChunkMesh);
    //if (Continuity < 1) {
    //    __debugbreak();
    //}
    //Continuity = MeshChunk(&Chunk2, &Chunk2.ChunkMesh);
    //if (Continuity < 1) {
    //    __debugbreak();
    //}


    gFreeCamera.target     = { 0,0,0 };
    gFreeCamera.position   = { 0,20,-20 };
    gFreeCamera.projection = CAMERA_PERSPECTIVE;
    gFreeCamera.fovy       = 90;
    gFreeCamera.up         = { 0,1,0 };

    //UploadMesh(&Chunk.ChunkMesh, FALSE);
    //UploadMesh(&Chunk2.ChunkMesh, FALSE);

    //Model modelChunk = LoadModelFromMesh(Chunk.ChunkMesh);
    //modelChunk.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    //Model modelChunk2 = LoadModelFromMesh(Chunk2.ChunkMesh);
    //modelChunk2.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    SCHAR Text[256] = { 0 };

    //SetTraceLogLevel(LOG_NONE);

    PHYSICALOBJECT objPlayer = { 0 };
    objPlayer.Position.x = 0;
    objPlayer.Position.y = 400;
    objPlayer.Position.z = 20;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        const SFLOAT32 Cd = 1.05;
        const SFLOAT32 kGravity = 32.1741;
        SFLOAT32 area = 4;
        SFLOAT32 weight = 200;
        SFLOAT32 mass = weight / kGravity;
        SFLOAT32 airdensity = 0.0765;
        SFLOAT32 drag = Cd *
            (
                (airdensity * pow(objPlayer.Velocity.y, 2))
                / 2
                )
            * area;
        //SFLOAT32 drag = (1 / 2) * Cd * area * airdensity * pow(objPlayer.Velocity.y, 2);


        objPlayer.Velocity.y -= ((weight - drag)/ mass);
        



        if (World.ChunkList.count(floor(objPlayer.Position.x / CHUNK_SIZE)) > 0 && World.ChunkList[floor(objPlayer.Position.x / CHUNK_SIZE)].count(floor(objPlayer.Position.y / CHUNK_SIZE)) > 0 && World.ChunkList[floor(objPlayer.Position.x / CHUNK_SIZE)][floor(objPlayer.Position.y / CHUNK_SIZE)].count(floor(objPlayer.Position.z / CHUNK_SIZE)) > 0) {

            Player.pCurrentChunk = World.ChunkList[floor(objPlayer.Position.x / CHUNK_SIZE)][floor(objPlayer.Position.y / CHUNK_SIZE)][floor(objPlayer.Position.z / CHUNK_SIZE)];
        } else {
            Player.pCurrentChunk = NULL;
        }
        BLOCKENTRYDATA* pEntry = NULL;

        SFLOAT32 Blockx = ((SINT32)floor(objPlayer.Position.x) % CHUNK_SIZE);
        SFLOAT32 Blocky = ((SINT32)floor(objPlayer.Position.y) % CHUNK_SIZE);
        SFLOAT32 Blockz = ((SINT32)floor(objPlayer.Position.z) % CHUNK_SIZE);
        Blockx = Blockx >= 0 ? Blockx : CHUNK_SIZE + Blockx;
        Blocky = Blocky >= 0 ? Blocky : CHUNK_SIZE + Blocky;
        Blockz = Blockz >= 0 ? Blockz : CHUNK_SIZE + Blockz;
        
        if (Player.pCurrentChunk != NULL) {
            if (
                (pEntry = worldGetBlockDataNearSafe(&World, Player.pCurrentChunk, Blockx, Blocky - 1, Blockz)) != NULL && pEntry->TypeId != 0) {
                objPlayer.Velocity.y = 0;
            }
            if ((pEntry = worldGetBlockDataNearSafe(&World, Player.pCurrentChunk, Blockx, Blocky + 1, Blockz)) != NULL && pEntry->TypeId != 0
                && Player.pCurrentChunk->BlockData[(SINT32)Blockx][(SINT32)Blocky][(SINT32)Blockz].TypeId != 0) {
                objPlayer.Position.y += 2;
            }
        }
        
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            objPlayer.Position.y -= 1;
        }
        if (IsKeyDown(KEY_SPACE)) {
            objPlayer.Position.y += 1;
        }
        if (IsKeyDown(KEY_HOME)) {
            objPlayer.Position.y = 400;
        }

        Vector3 NewPosition = objectGetPredictedFuturePosition_Normalized(&objPlayer, 1);
        SFLOAT32 Dist = 0;
        /*/
        while (true) {
            NewPosition = objectGetPredictedFuturePosition_Normalized(&objPlayer, Dist);

            //if(Dist > 1/GetFrameTime()) {
            //    break;
            //}

            if (Vector3Distance(objPlayer.Position, NewPosition) > Vector3Distance(objPlayer.Position, objectGetPredictedFuturePosition(&objPlayer, 1)) * GetFrameTime()) {
                break;
            }

            pEntry = worldGetBlockDataNearSafe(&World, Player.pCurrentChunk, NewPosition.x, NewPosition.y, NewPosition.z);
            if (pEntry != NULL) {
                if (pEntry->TypeId != 0) {
                    break;
                }
            } else {
                objPlayer.Position = NewPosition;
            }
            Dist++;
            //objPlayer.Velocity = Vector3Subtract(objPlayer.Velocity, NewPosition);
        }
        */
        SFLOAT32 kTimesteps = 2;


        NewPosition = objPlayer.Position;
        /*

        Vector3 FinalPosition = objectGetPredictedFuturePosition(&objPlayer, GetFrameTime());
        SFLOAT32 distPerStep = (Vector3Distance(objPlayer.Position, FinalPosition)) / kTimesteps;

        for (SINT32 iSteps = 0; iSteps < kTimesteps; iSteps++) {
            //NewPosition = objectGetPredictedFuturePosition_Normalized(&objPlayer, iSteps * distPerStep);
            NewPosition = Vector3Add(objPlayer.Position, Vector3Scale(objPlayer.Velocity, (1 / kTimesteps) * GetFrameTime()));

            pEntry = worldGetBlockDataNearSafe(&World, Player.pCurrentChunk, NewPosition.x, NewPosition.y, NewPosition.z); 

            if (pEntry != NULL && pEntry->TypeId != 0) {
                objPlayer.Velocity = { 0,0,0 };
                // Go far as possible
                break;
            } else {
                objPlayer.Position = NewPosition;
            }
            

        }*/
        /*
        for (SINT32 iSteps = 0; iSteps < kTimesteps; iSteps++) {
            NewPosition = Vector3Add(objPlayer.Position, Vector3Scale(objPlayer.Velocity, GetFrameTime()/kTimesteps));

            pEntry = worldGetBlockDataNearSafe(&World, Player.pCurrentChunk, NewPosition.x, NewPosition.y, NewPosition.z);

            if (pEntry != NULL && pEntry->TypeId != 0) {
                objPlayer.Velocity = { 0,0,0 };
                // Go far as possible
                break;
            }
            else {
                objPlayer.Position = NewPosition;
            }

        }*/


        // Cast ray between current and theoretical next position
        // if no collisions, set 
        // else set furtherest possible and calculate kinetic energy impact
        objPlayer.Position = Vector3Add(objPlayer.Position, Vector3Scale(objPlayer.Velocity,GetFrameTime()));



        //else {
        //    objPlayer.Position.x += objPlayer.Velocity.x;
        //    objPlayer.Position.y += objPlayer.Velocity.y;
        //    objPlayer.Position.z += objPlayer.Velocity.z;
        //}


        // hakk hakk hakk hakk hakk hakk hakk hakk hakk
        gFreeCamera.target.y -= gFreeCamera.position.y - (objPlayer.Position.y + 1);

        objPlayer.Position.x = gFreeCamera.position.x;
        gFreeCamera.position.y = objPlayer.Position.y + 1;
        objPlayer.Position.z = gFreeCamera.position.z;
        // hakk hakk hakk hakk hakk hakk hakk hakk hakk

        UpdateCamera(&gFreeCamera, CAMERA_FREE);

        //Chunk.BlockData[rand() % CHUNK_SIZE][rand() % CHUNK_SIZE][rand() % CHUNK_SIZE].TypeId = 1;
        //Chunk2.BlockData[rand() % CHUNK_SIZE][rand() % CHUNK_SIZE][rand() % CHUNK_SIZE].TypeId = 0;

        worldLoadChunksPerCoords(&World, Player.pCurrentChunk->position.x, Player.pCurrentChunk->position.y, Player.pCurrentChunk->position.z, Client.RenderDistance.x, Client.RenderDistance.y, Client.RenderDistance.z);

        worldLoadSomeChunks(&World, Player.pCurrentChunk->position.x, Player.pCurrentChunk->position.y, Player.pCurrentChunk->position.z);


        for (auto it0 = World.ChunkList.begin(); it0 != World.ChunkList.end(); ++it0) {
                for (auto it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
                    for (auto it = it1->second.begin(); it != it1->second.end(); ++it) {
                    if (it->second->bChanged == TRUE) {
                        if (it->second->bRender == TRUE) {
                            Continuity = MeshChunk(it->second, &it->second->ChunkMesh);
                            if (Continuity < 1) {
                                __debugbreak();
                            }

                            if (it->second->countMeshings == 0) {
                                //UpdateMeshBuffer(it->second->ChunkMesh, 0, );
                                UploadMesh(&it->second->ChunkMesh, FALSE);

                                it->second->ChunkModel = LoadModelFromMesh(it->second->ChunkMesh);
                                it->second->ChunkModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
                            }

                            it->second->countMeshings++;
                            // it->second->bChanged = FALSE;
                        }
                    }
                }
            }
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GRAY);

            BeginMode3D(gFreeCamera);
                
                
                DrawGrid(32,1);

                //DrawMesh(meshChunk, matChunk, mtxChunk);
                worldRender(&World);
                
                DrawCube(objPlayer.Position, 2, 6, 2, WHITE);


            EndMode3D();
            Vector3 CameraForward = GetCameraForward(&gFreeCamera);

            //SFLOAT32 Blockx = (SFLOAT32)(((SINT32)floor(objPlayer.Position.x) % CHUNK_SIZE));
            //SFLOAT32 Blocky = (SFLOAT32)(((SINT32)floor(objPlayer.Position.y) % CHUNK_SIZE));
            //SFLOAT32 Blockz = (SFLOAT32)(((SINT32)floor(objPlayer.Position.z) % CHUNK_SIZE));
            //Blockx = Blockx > 0 ? Blockx : CHUNK_SIZE + Blockx;
            //Blocky = Blocky > 0 ? Blocky : CHUNK_SIZE + Blocky;
            //Blockz = Blockz > 0 ? Blockz : CHUNK_SIZE + Blockz;


            //snprintf(Text, 255, "Forward %f, %f, %f \n Pos %f %f %f\n Chunk 1\n\tVerts[%i]\r\tInd[%i]", CameraForward.x, CameraForward.y, CameraForward.z, gFreeCamera.position.x, gFreeCamera.position.y, gFreeCamera.position.z, (SINT32)Chunk.statVertices, (SINT32)Chunk.statIndices);
            snprintf(Text, 255, "Pos %f %f %f\n Vel %f %f %f\n W: %f\n D: %f\nchunk: %f %f %f\nblock: %f %f %f\n", 
                objPlayer.Position.x, objPlayer.Position.y, objPlayer.Position.z, objPlayer.Velocity.x, objPlayer.Velocity.y, objPlayer.Velocity.z, weight, drag,
                floor(objPlayer.Position.x / CHUNK_SIZE), floor(objPlayer.Position.y / CHUNK_SIZE), floor(objPlayer.Position.z / CHUNK_SIZE),
                Blockx, Blocky, Blockz
            );

            DrawText(Text, 190, 200, 20, BLACK);

            DrawFPS(0, 0);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
