#pragma once
#include <fbxsdk.h>
#include <vector>
#include <unordered_map>
//#include <Core/Mate/Serializer/Serializer.h>
//#include <Resource/ResourceType/Data/MeshData.h>

namespace FakeReal
{
    struct Vertex
    {
        FbxVector4 position;
        FbxVector4 normal;
        FbxVector4 tangent;
        FbxVector4 bitangent;
        FbxVector2 uv;
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialIndex;
    };

    struct FbxTextureData
    {
        std::string m_Name;
        std::string m_Path;
        std::string m_RelativePath;
        std::string m_TypeName = "texture_diffuse";
    };

    struct FbxKeyFrameData
    {
        FbxVector4 tran;
        FbxQuaternion rot;
        FbxVector4 scale;
        FbxAMatrix globalPose;
        double keyTime;
    };

    struct FbxAnimData
    {
        std::unordered_map<std::string, std::vector<FbxKeyFrameData>> data;
    };
    class FbxConverter
    {
    public:
        enum // EXPORT TYPE
        {
            ET_STATIC_MESH,
            ET_SKELECTON_MESH,
            ET_ACTION,
            ET_MAX
        };

        FbxConverter(int argc, char** argv);
        ~FbxConverter();

        bool ExportFile();
        void InitFbxObjects();
        bool CreateScene();

    private:
        void DestroySdkObjects();
        bool LoadScene(const std::string& file);

    private:
        void ProcessMesh(FbxNode* pNode);
        void GetMesh(FbxNode* pNode);
        // void ProcessSkeleton(FbxNode* pNode, Bone* pParentBone = nullptr);
        // void GetOffsetMatrix(FbxSkin* pSkin);
        void LoadTexture(FbxMesh* pMesh, std::vector<FbxTextureData>& outList);
        // void FbxMatToGlmMat(glm::mat4& out, const FbxAMatrix& in);
        void BoneSkin(FbxSkin* pSkin, std::vector<std::string>& Bones, std::vector<float>& Weights, int ctrlPointIndex);
        void GetAnim(FbxNode* pNode);

        void WriteMeshToFile();
        // void WriteSkeletonMeshToFile();
        // void WriteAnimToFile();
        void ClearAllInfo();
        void GetTriangleMaterialIndex(FbxMesh* pMesh, int triangleCount, std::vector<int>& TriangleMaterialIndexList);
        void GetTriangleSmGroupIndex(FbxMesh* pMesh, int triangleCount, std::vector<int>& TriangleSmGroupIndexList);
        void ReadVertex(FbxMesh* pMesh, int ctrlPointIndex, FbxVector4& V);
        void ReadNormal(FbxMesh* pMesh, int ctrlPointIndex, int vertexCounter, FbxVector4& N);
        void ReadUV(FbxMesh* pMesh, int ctrlPointIndex, int triangleIndex, int triangleVertexIndex, int uvIndex, FbxVector2& UV);
        void CreateStaticMesh(const std::string& name, int UVNum, bool hasSkin, uint32_t materialIndex);

    private:
        FbxManager* m_pManager;
        FbxScene* m_pScene;
        std::string m_File;
        std::string m_OutPut;
        std::string m_DefaultName;
        bool m_LoadSceneResult;
        int m_SkeletonCount;
        unsigned int mExportType;

    private:
        FbxAnimData m_Anim;

        //std::unordered_map<std::string, Mesh> mMeshes;
        std::vector<Mesh> mMeshes;
        std::unordered_map<uint32_t, std::vector<FbxTextureData>> mTextures;
        // MeshData* m_pMeshData;
        // GeometryNode* m_pGeoNode;
        // Skeleton* m_pSkeleton;
        // Animation* m_pAnimation;

        std::vector<FbxVector4> mVertexArray;
        std::vector<FbxVector4> mNormalArray;
        std::vector<FbxVector2> mTexCoordArray[2]; // ���2��UV
        std::vector<unsigned int> mIndexArray;
        std::vector<unsigned int> mVertexSmGroupArray;
        // std::vector<std::string> mMeshBoneNameArray;
        // std::vector<glm::ivec4> mBoneIndexArray;//�������洢����mMeshBoneNameArray���±�
        // std::vector<glm::vec4> mBoneWeightArray;
    };
} // namespace FakeReal