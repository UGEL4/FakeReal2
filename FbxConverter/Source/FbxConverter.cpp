#include "FbxConverter.h"
// #include <Core/Base/Macro.h>
#include <fstream>
#include <sstream>
#include <format>
// #include <Resource/AssetManager/AssetManager.h>
// #include <_generated/MeshData_gen.h>

namespace FakeReal
{
    FbxConverter::FbxConverter(int argc, char** argv)
        : m_pManager(nullptr)
        , m_pScene(nullptr)
        , m_LoadSceneResult(false)
        , m_SkeletonCount(0)
        , mExportType(ET_MAX)
    //, m_pMeshNode(nullptr)
    //, m_pGeoNode(nullptr)
    //, m_pSkeleton(nullptr)
    //, m_pAnimation(nullptr)
    {
        m_File       = argv[0];
        m_OutPut     = argv[1];
        size_t index = m_File.rfind("\\");
        if (index == std::string::npos)
        {
            // ������б��
            index = m_File.rfind("/");
        }
        m_DefaultName = m_File.substr(index + 1);

        if (argc < 3)
            mExportType = ET_STATIC_MESH;
        else
        {
            std::string exportArg(argv[2]);
            if (exportArg == "-s")
                mExportType = ET_STATIC_MESH;
            else if (exportArg == "-d")
                mExportType = ET_SKELECTON_MESH;
            else if (exportArg == "-a")
                mExportType = ET_ACTION;
            else
                mExportType = ET_STATIC_MESH;
        }
    }

    FbxConverter::~FbxConverter()
    {
        DestroySdkObjects();
        /*if (m_pMeshNode)
            delete m_pMeshNode;*/
    }

    bool FbxConverter::ExportFile()
    {
        InitFbxObjects();
        CreateScene();
        return true;
    }

    void FbxConverter::InitFbxObjects()
    {
        // The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
        m_pManager = FbxManager::Create();
        if (!m_pManager)
        {
            FBXSDK_printf("Error: Unable to create FBX Manager!\n");
            exit(1);
        }
        else
        {
            FBXSDK_printf("Autodesk FBX SDK version %s\n", m_pManager->GetVersion());
        }

        // Create an IOSettings object. This object holds all import/export settings.
        FbxIOSettings* ios = FbxIOSettings::Create(m_pManager, IOSROOT);
        m_pManager->SetIOSettings(ios);

        // Load plugins from the executable directory (optional)
        /*FbxString lPath = FbxGetApplicationDirectory();
        m_pManager->LoadPluginsDirectory(lPath.Buffer());*/

        // Create an FBX scene. This object holds most objects imported/exported from/to files.
        m_pScene = FbxScene::Create(m_pManager, "My Scene");
        if (!m_pScene)
        {
            FBXSDK_printf("Error: Unable to create FBX scene!\n");
            exit(1);
        }
    }

    bool FbxConverter::CreateScene()
    {
        bool lResult = false;
        FBXSDK_printf("\n\nFile: %s\n\n", m_File.c_str());
        m_LoadSceneResult = lResult = LoadScene(m_File);
        if (lResult == false)
        {
            FBXSDK_printf("\n\nAn error occurred while loading the scene...");
            return false;
        }
        else
        {
            // �����λ�
            /* FbxGeometryConverter converter(m_pManager);
            converter.Triangulate(m_pScene, true); */

            // DisplayMetaData();
            // �����ڵ�
            FbxNode* pRootNode = m_pScene->GetRootNode();
            if (pRootNode)
            {
                if (mExportType == ET_STATIC_MESH)
                {
                    mMeshes.clear();
                    // m_pMeshData = new MeshData;
                    // m_pGeoNode = new GeometryNode;
                    // m_pMeshNode->AddChild(m_pGeoNode);
                    ProcessMesh(pRootNode);
                }
                else if (mExportType == ET_SKELECTON_MESH)
                {
                    // m_pMeshNode = new SkeletonMeshNode;
                    // m_pSkeleton = new Skeleton;
                    // ProcessSkeleton(pRootNode);
                    // m_pSkeleton->CreateBoneArray();

                    // m_pGeoNode = new GeometryNode;
                    // m_pMeshNode->AddChild(m_pGeoNode);
                    // ProcessMesh(pRootNode);
                }
                else if (mExportType == ET_ACTION)
                {
                    // m_pSkeleton = new Skeleton;
                    // ProcessSkeleton(pRootNode);
                    // m_pSkeleton->CreateBoneArray();
                    // GetAnim(pRootNode);
                }
            }

            // д���ļ�
            if (mExportType == ET_STATIC_MESH)
            {
                if (!mMeshes.empty())
                    WriteMeshToFile();
                else
                {
                    FBXSDK_printf("\n\nStaticMesh Export Error!...\n\n");
                    return false;
                }
            }
            /*else if (mExportType == ET_SKELECTON_MESH)
            {
                if (m_pMeshNode && m_pSkeleton)
                    WriteSkeletonMeshToFile();
                else
                {
                    FBXSDK_printf("\n\nSkeletonMesh Export Error!...\n\n");
                    return false;
                }
            }
            else if (mExportType == ET_ACTION)
            {
                if (m_pSkeleton)
                    WriteAnimToFile();
                else
                {
                    FBXSDK_printf("\n\nAnimation Export Error!...\n\n");
                    return false;
                }
            }*/
        }

        FBXSDK_printf("\n\nDone!...\n\n");

        return true;
    }

    void FbxConverter::DestroySdkObjects()
    {
        // Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
        if (m_pManager) m_pManager->Destroy();
        if (m_LoadSceneResult) FBXSDK_printf("Program Success!\n");
    }

    bool FbxConverter::LoadScene(const std::string& file)
    {
        int lFileMajor, lFileMinor, lFileRevision;
        int lSDKMajor, lSDKMinor, lSDKRevision;
        // int lFileFormat = -1;
        int lAnimStackCount;
        bool lStatus;
        char lPassword[1024];

        // Get the file version number generate by the FBX SDK.
        FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);
        // create importer
        FbxImporter* pImporter = FbxImporter::Create(m_pManager, "");
        // Initialize the importer by providing a filename.
        const bool importerStatus = pImporter->Initialize(file.c_str(), -1, m_pManager->GetIOSettings());
        pImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
        if (!importerStatus)
        {
            FbxString error = pImporter->GetStatus().GetErrorString();
            FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
            FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

            if (pImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
            {
                FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
                FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", file.c_str(), lFileMajor, lFileMinor, lFileRevision);
            }

            return false;
        }

        FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
        FbxIOSettings* pIOSettings = m_pManager->GetIOSettings();
        if (pImporter->IsFBX())
        {
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", file.c_str(), lFileMajor, lFileMinor, lFileRevision);

            // From this point, it is possible to access animation stack information without
            // the expense of loading the entire file.

            FBXSDK_printf("Animation Stack Information\n");

            lAnimStackCount = pImporter->GetAnimStackCount();

            FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
            FBXSDK_printf("    Current Animation Stack: \"%s\"\n", pImporter->GetActiveAnimStackName().Buffer());
            FBXSDK_printf("\n");

            for (int i = 0; i < lAnimStackCount; i++)
            {
                FbxTakeInfo* lTakeInfo = pImporter->GetTakeInfo(i);

                FBXSDK_printf("    Animation Stack %d\n", i);
                FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
                FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

                // Change the value of the import name if the animation stack should be imported
                // under a different name.
                FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

                // Set the value of the import state to false if the animation stack should be not
                // be imported.
                FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
                FBXSDK_printf("\n");
            }

            // Set the import states. By default, the import states are always set to
            // true. The code below shows how to change these states.
            /*pIOSettings->SetBoolProp(IMP_FBX_MATERIAL, true);
            pIOSettings->SetBoolProp(IMP_FBX_TEXTURE, true);
            pIOSettings->SetBoolProp(IMP_FBX_LINK, true);
            pIOSettings->SetBoolProp(IMP_FBX_SHAPE, true);
            pIOSettings->SetBoolProp(IMP_FBX_GOBO, true);
            pIOSettings->SetBoolProp(IMP_FBX_ANIMATION, true);
            pIOSettings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);*/
        }

        // Import the scene.
        lStatus = pImporter->Import(m_pScene);
        if (lStatus == false && pImporter->GetStatus() == FbxStatus::ePasswordError)
        {
            FBXSDK_printf("Please enter password: ");

            lPassword[0] = '\0';

            FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
            scanf("%s", lPassword);
            FBXSDK_CRT_SECURE_NO_WARNING_END

            FbxString lString(lPassword);

            pIOSettings->SetStringProp(IMP_FBX_PASSWORD, lString);
            pIOSettings->SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

            lStatus = pImporter->Import(m_pScene);

            if (lStatus == false && pImporter->GetStatus() == FbxStatus::ePasswordError)
            {
                FBXSDK_printf("\nPassword is wrong, import aborted.\n");
            }
        }

        if (!lStatus || (pImporter->GetStatus() != FbxStatus::eSuccess))
        {
            FBXSDK_printf("********************************************************************************\n");
            if (lStatus)
            {
                FBXSDK_printf("WARNING:\n");
                FBXSDK_printf("   The importer was able to read the file but with errors.\n");
                FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
            }
            else
            {
                FBXSDK_printf("Importer failed to load the file!\n\n");
            }

            if (pImporter->GetStatus() != FbxStatus::eSuccess)
                FBXSDK_printf("   Last error message: %s\n", pImporter->GetStatus().GetErrorString());

            /*FbxArray<FbxString*> history;
            pImporter->GetStatus().GetErrorStringHistory(history);
            if (history.GetCount() > 1)
            {
                FBXSDK_printf("   Error history stack:\n");
                for (int i = 0; i < history.GetCount(); i++)
                {
                    FBXSDK_printf("      %s\n", history[i]->Buffer());
                }
            }
            FbxArrayDelete<FbxString*>(history);*/
            FBXSDK_printf("********************************************************************************\n");
        }

        // Destroy the importer.
        pImporter->Destroy();

        return lStatus;
    }

    void FbxConverter::ProcessMesh(FbxNode* pNode)
    {
        if (!pNode)
        {
            FBXSDK_printf("ERROR: function:ProcessMesh(FbxNode* pNode) param FbxNode* pNode = NULL.\n");
            return;
        }
        FbxNodeAttribute::EType lAttributeType;
        if (pNode->GetNodeAttribute())
        {
            lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();
            if (lAttributeType == fbxsdk::FbxNodeAttribute::eMesh)
            {
                GetMesh(pNode);
            }
        }
        else
        {
            FBXSDK_printf("NULL Node Attribute\n\n");
        }

        for (int i = 0; i < pNode->GetChildCount(); i++)
        {
            ProcessMesh(pNode->GetChild(i));
        }
    }

    void FbxConverter::GetMesh(FbxNode* pNode)
    {
        // ��������
        FbxMesh* pMesh    = (FbxMesh*)pNode->GetNodeAttribute();
        FbxSkin* pFBXSkin = nullptr;

        FbxGeometryConverter converter(m_pManager);
        FbxNodeAttribute* attrib = converter.Triangulate(pMesh, true);
        if (attrib && attrib->GetAttributeType() == fbxsdk::FbxNodeAttribute::eMesh)
        {
            pMesh = (FbxMesh*)attrib;
        }
        else
        {
            return;
        }

        /*if (mExportType == ET_SKELECTON_MESH)
        {
            int deformerCount = pMesh->GetDeformerCount();
            for (int i = 0; i < deformerCount; i++)
            {
                FbxDeformer* pDeformer = pMesh->GetDeformer(i);
                if (!pDeformer)
                    continue;
                if (pDeformer->GetDeformerType() == FbxDeformer::eSkin)
                {
                    pFBXSkin = (FbxSkin*)pDeformer;
                    break;
                }
            }
            if (!pFBXSkin)
            {
                FBXSDK_printf("ERROR: Mesh has no skin......\n");
                return;
            }
            GetOffsetMatrix(pFBXSkin);
        }*/

        FBXSDK_printf("ProcessMesh......%d\n", pMesh->GetControlPointsCount());
        int triangleCount = pMesh->GetPolygonCount();
        FBXSDK_printf("			TriangleCount = %d \n", triangleCount);
        std::vector<int> TriangleMaterialIndex;
        TriangleMaterialIndex.resize(triangleCount, 0);
        GetTriangleMaterialIndex(pMesh, triangleCount, TriangleMaterialIndex);

        std::vector<int> TriangleSmGroupIndex;
        TriangleSmGroupIndex.resize(triangleCount, 0);
        GetTriangleSmGroupIndex(pMesh, triangleCount, TriangleSmGroupIndex);

        int MaterialCount = pNode->GetMaterialCount();
        int UVNum         = pMesh->GetElementUVCount();
        for (int k = 0; (k == 0 || k < MaterialCount); k++)
        {
            ClearAllInfo();

            for (int i = 0; i < triangleCount; ++i)
            {
                if (TriangleMaterialIndex[i] == k)
                {
                    for (int j = 0; j < 3; j++)
                    {
                        // vertex
                        FbxVector4 V;
                        int controlPointIndex = pMesh->GetPolygonVertex(i, j);
                        ReadVertex(pMesh, controlPointIndex, V);

                        // normal
                        FbxVector4 N;
                        ReadNormal(pMesh, controlPointIndex, j + 3 * i, N);

                        // UV
                        std::vector<FbxVector2> UVArray;
                        for (int uv = 0; uv < UVNum; uv++)
                        {
                            FbxVector2 UV;
                            ReadUV(pMesh, controlPointIndex, i, j, uv, UV);
                            UVArray.emplace_back(UV);
                        }

                        int f = 0;
                        for (f = 0; f < mVertexArray.size(); ++f)
                        {
                            if (V == mVertexArray[f])
                            {
                                if (TriangleSmGroupIndex[i] == mVertexSmGroupArray[f])
                                {
                                    int uvChannel = 0;
                                    for (; uvChannel < UVNum; ++uvChannel)
                                    {
                                        if (UVArray[uvChannel] == mTexCoordArray[uvChannel][f])
                                        {
                                            continue;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }

                                    if (uvChannel == UVNum)
                                        break;
                                }
                            }
                        }
                        // ��û���������v�����Ӷ���v
                        if (f == mVertexArray.size())
                        {
                            mVertexArray.emplace_back(V);
                            mNormalArray.emplace_back(N);
                            for (int uvChannel = 0; uvChannel < UVNum && uvChannel < 2; uvChannel++)
                            {
                                mTexCoordArray[uvChannel].emplace_back(UVArray[uvChannel]);
                            }
                            mVertexSmGroupArray.emplace_back(TriangleSmGroupIndex[i]);

                            /*if (pFBXSkin)
                            {
                                //���Ӱ�쵱ǰ��������й�����Ȩ��
                                std::vector<std::string> BoneTemp;
                                std::vector<float> WeightTemp;
                                BoneSkin(pFBXSkin, BoneTemp, WeightTemp, controlPointIndex);

                                //û�й���Ӱ���������
                                if (BoneTemp.size() == 0)
                                    return;

                                //���Ӱ����������Ĺ�ͷ����4�������Ȩ��С�Ĺ�ͷȥ�������ٵ�4����ͷ
                                while (BoneTemp.size() > 4)
                                {
                                    float MinWeight = WeightTemp[0];
                                    size_t MinWeightIndex = 0;
                                    for (size_t j = 1; j < WeightTemp.size(); j++)
                                    {
                                        if (WeightTemp[j] < MinWeight)
                                        {
                                            MinWeight = WeightTemp[j];
                                            MinWeightIndex = j;
                                        }

                                    }
                                    BoneTemp.erase(BoneTemp.begin() + MinWeightIndex);
                                    WeightTemp.erase(WeightTemp.begin() + MinWeightIndex);
                                }
                                //���¼���Ȩ��
                                float TotleWeight = 0.0;
                                for (size_t i = 0; i < WeightTemp.size(); i++)
                                {
                                    TotleWeight += WeightTemp[i];
                                }
                                for (size_t i = 0; i < WeightTemp.size(); i++)
                                {
                                    WeightTemp[i] = WeightTemp[i] / TotleWeight;
                                }
                                //�ѹ�������Mesh�����б�����Ϊ������������������������б���������Ȩ��
                                //һ���������֧��4��������������vec4
                                glm::ivec4 BoneIndex(-1, -1, -1, -1);//-1��ʾû�й���Ӱ��
                                glm::vec4 BoneWeight(0.0f, 0.0f, 0.0f, 0.0f);
                                for (size_t i = 0; i < BoneTemp.size(); i++)
                                {
                                    Bone* pBone = m_pSkeleton->GetBone(BoneTemp[i]);
                                    if (!pBone)
                                        return;
                                    size_t j = 0;
                                    for (; j < mMeshBoneNameArray.size(); j++)
                                    {
                                        if (BoneTemp[i] == mMeshBoneNameArray[j])
                                            break;
                                    }
                                    if (j == mMeshBoneNameArray.size())
                                    {
                                        mMeshBoneNameArray.emplace_back(BoneTemp[i]);
                                    }
                                    BoneIndex[i] = j;
                                    BoneWeight[i] = WeightTemp[i];
                                }
                                mBoneIndexArray.emplace_back(BoneIndex);
                                mBoneWeightArray.emplace_back(BoneWeight);
                            }*/
                        }

                        mIndexArray.emplace_back(f);

                    } // for j = 2
                }
            } // for i = 0

            // ��������
            CreateStaticMesh(pMesh->GetName(), UVNum, (pFBXSkin != nullptr));
            //texture
            LoadTexture(pMesh, k);
        } // for k = 0

        // texture
        /* std::vector<FbxTextureData> outList;
        LoadTexture(pMesh, outList); */

        // m_MeshList.push_back(meshData);
    }

    /*void FbxConverter::ProcessSkeleton(FbxNode* pNode, Bone* pParentBone)
    {
        Bone* pBone = nullptr;
        if (pNode->GetNodeAttribute())
        {
            switch (pNode->GetNodeAttribute()->GetAttributeType())
            {
            case FbxNodeAttribute::eSkeleton:
            {

                FbxTimeSpan timeSpan;
                m_pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(timeSpan);
                FbxTime start = timeSpan.GetStart();
                FbxTime end = timeSpan.GetStop();

                pBone = new Bone(pNode->GetName(), m_SkeletonCount);
                m_SkeletonCount++;

                FbxAMatrix Combine = pNode->EvaluateLocalTransform(start);

                glm::mat4 Mat;
                FbxMatToGlmMat(Mat, Combine);
                pBone->SetLocalMat(Mat);
                pBone->SetLocalScale({ 1.0f, 1.0f, 1.0f });
                if (pParentBone)
                {
                    pParentBone->AddChild(pBone);
                }
                else
                {
                    m_pSkeleton->AddChild(pBone);
                }
            }
            break;
            }
        }
        for (int i = 0; i < pNode->GetChildCount(); ++i)
        {
            ProcessSkeleton(pNode->GetChild(i), pBone);
        }
    }*/

    /*void FbxConverter::GetOffsetMatrix(FbxSkin* pSkin)
    {
        int clusterCount = pSkin->GetClusterCount();
        for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
        {
            FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);
            if (!pCluster)
            {
                continue;
            }
            const FbxNode* pLinkNode = pCluster->GetLink();
            Bone* pBone = m_pSkeleton->GetBone(pLinkNode->GetName());
            assert(pBone);
            FbxAMatrix vertexBindMatrix, boneBindMatrix;
            pCluster->GetTransformMatrix(vertexBindMatrix);
            pCluster->GetTransformLinkMatrix(boneBindMatrix);
            const FbxVector4 lT = pLinkNode->GetGeometricTranslation(FbxNode::eSourcePivot);
            const FbxVector4 lR = pLinkNode->GetGeometricRotation(FbxNode::eSourcePivot);
            const FbxVector4 lS = pLinkNode->GetGeometricScaling(FbxNode::eSourcePivot);
            FbxAMatrix geomMatrix(lT, lR, lS);
            FbxAMatrix Combine = boneBindMatrix.Inverse() * vertexBindMatrix * geomMatrix;
            glm::mat4 offsetMatrix(1.0f);
            FbxMatToGlmMat(offsetMatrix, Combine);
            pBone->SetInvBindPos(offsetMatrix);
        }
    }*/

    void FbxConverter::LoadTexture(FbxMesh* pMesh, uint32_t materialIndex)
    {
        FbxNode* pNode = pMesh->GetNode();
        int materialCount = pNode->GetMaterialCount();
        if (!materialCount || materialCount <= materialIndex)
        {
            return;
        }

        FbxSurfaceMaterial* surfaceMat = pNode->GetMaterial(materialIndex);
        if (!surfaceMat) return;

        auto iter = mTextures.find(surfaceMat);
        if (iter != mTextures.end()) return;

        auto& texList = mTextures[surfaceMat];
        mMaterialCount++;
        for (int textureLayerIndex = 0; textureLayerIndex < FbxLayerElement::sTypeTextureCount; textureLayerIndex++)
        {
            FbxProperty prop = surfaceMat->FindProperty(FbxLayerElement::sTextureChannelNames[textureLayerIndex]);
            if (prop.IsValid())
            {
                int textureCount = prop.GetSrcObjectCount<FbxTexture>();
                for (int t = 0; t < textureCount; t++)
                {
                    FbxTexture* tex = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(t));
                    if (tex)
                    {
                        FbxFileTexture* fileTex = FbxCast<FbxFileTexture>(tex);
                        std::string name        = fileTex->GetFileName();
                        std::string type(FbxLayerElement::sTextureChannelNames[textureLayerIndex]);

                        FbxTextureData data = {
                            .m_Name         = name,
                            .m_Path         = name,
                            .m_RelativePath = name,
                            .m_TypeName     = type
                        };
                        texList.emplace_back(data);
                    }
                }
            }
        }

        /* int triangleCount = pMesh->GetPolygonCount();
        for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
        {
            int materialIndex = pMaterial->GetIndexArray().GetAt(triangleIndex);

            TriangleMaterialIndexList[triangleIndex] = materialIndex;
        } */
    }

    /*void FbxConverter::FbxMatToGlmMat(glm::mat4& out, const FbxAMatrix& in)
    {
        FbxVector4 v = in.GetRow(0);
        out[0] = glm::vec4((float)v[0], (float)v[1], (float)v[2], (float)v[3]);

        v = in.GetRow(1);
        out[1] = glm::vec4((float)v[0], (float)v[1], (float)v[2], (float)v[3]);

        v = in.GetRow(2);
        out[2] = glm::vec4((float)v[0], (float)v[1], (float)v[2], (float)v[3]);

        v = in.GetRow(3);
        out[3] = glm::vec4((float)v[0], (float)v[1], (float)v[2], (float)v[3]);
    }*/

    // ctrlPointIndex: vertex index
    void FbxConverter::BoneSkin(FbxSkin* pSkin, std::vector<std::string>& Bones, std::vector<float>& Weights, int ctrlPointIndex)
    {
        int clusterCount = pSkin->GetClusterCount();
        for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
        {
            FbxCluster* pCluster         = pSkin->GetCluster(clusterIndex);
            const FbxNode* pLinkNode     = pCluster->GetLink();
            int controlPointIndicesCount = pCluster->GetControlPointIndicesCount();
            int* indices                 = pCluster->GetControlPointIndices();
            double* fbxWeights           = pCluster->GetControlPointWeights();
            for (int i = 0; i < controlPointIndicesCount; i++)
            {
                if (indices[i] == ctrlPointIndex) // ctrlPointIndex: vertex index
                {
                    size_t j = 0;
                    for (j = 0; j < Bones.size(); j++)
                    {
                        if (pLinkNode->GetName() == Bones[j])
                        {
                            Weights[j] += (float)fbxWeights[i];
                        }
                    }
                    if (j == Bones.size())
                    {
                        // ���ӹ�����Ȩ��
                        Bones.emplace_back(pLinkNode->GetName());
                        Weights.emplace_back((float)fbxWeights[i]);
                    }
                }
            }
        }
    }

    void FbxConverter::GetAnim(FbxNode* pNode)
    {
        if (!pNode)
        {
            return;
        }

        // FbxNodeAttribute::EType lAttributeType;
        if (pNode->GetNodeAttribute() == NULL)
        {
            FBXSDK_printf("NULL Node Attribute\n\n");
        }

        // const FbxLongLong ANIMATION_STEP_30 = 1539538600; // 30frames/seconds
        // const FbxLongLong ANIMATION_STEP_15 = 3079077200; // 30frames/seconds

        if (pNode->GetNodeAttribute())
        {
            FbxNodeAttribute::EType ATyep = pNode->GetNodeAttribute()->GetAttributeType();
            const char* NodeName          = pNode->GetName();
            switch (ATyep)
            {
                case FbxNodeAttribute::eNull:
                case FbxNodeAttribute::eSkeleton:
                {
                    std::string name            = pNode->GetName();
                    FbxAnimStack* currAnimStack = m_pScene->GetSrcObject<FbxAnimStack>(0);
                    FbxString animStackName     = currAnimStack->GetName();
                    FbxTakeInfo* takeInfo       = m_pScene->GetTakeInfo(animStackName);
                    FbxTime start               = takeInfo->mLocalTimeSpan.GetStart();
                    FbxTime end                 = takeInfo->mLocalTimeSpan.GetStop();
                    for (long long i = start.GetFrameCount(FbxTime::eFrames30); i <= end.GetFrameCount(FbxTime::eFrames30); ++i)
                    {
                        FbxTime currTime;
                        currTime.SetFrame(i, FbxTime::eFrames30);
                        FbxAMatrix Combine = pNode->EvaluateLocalTransform(currTime);
                        FbxKeyFrameData keyData;
                        keyData.tran       = Combine.GetT();
                        keyData.rot        = Combine.GetQ();
                        keyData.scale      = Combine.GetS();
                        keyData.globalPose = pNode->EvaluateGlobalTransform(currTime);
                        // keyData.keyTime = currTime.GetMilliSeconds() * 1.f;
                        keyData.keyTime = currTime.GetSecondDouble();
                        m_Anim.data[name].push_back(keyData);
                    }
                }
                break;
            }
        }

        for (int i = 0; i < pNode->GetChildCount(); ++i)
        {
            GetAnim(pNode->GetChild(i));
        }
    }

    void FbxConverter::WriteMeshToFile()
    {
        /*
        std::string& fileSuffix = Resource::GetFileSuffix(Resource::RT_STATIC_MODEL);
        Stream saveStream;
        saveStream.SetStreamFlag(Stream::AT_REGISTER);
        saveStream.ArchiveAll(m_pMeshNode);
        if (saveStream.NewSave((m_OutPut + m_DefaultName + "." + fileSuffix + ".assert").c_str()))
        {
            FBXSDK_printf("\n\n Write Mesh Done!...\n\n");
        }
        */
        /* for (auto& pair : mMeshes)
        {
            std::ofstream os((m_OutPut + pair.first + ".mesh.json").c_str());
            if (os.is_open())
            {
                JsonWriter writer;
                bool result = Serializer::Write(writer, pair.second);
                if (result)
                {
                    os << writer.GetString();
                }

                os.close();
            }
        } */

        std::stringstream out;
        out << "{\n"
               "    \"mMeshes\": [\n";
        for (size_t i = 0; i < mMeshes.size(); i++)
        {
            // mesh begin

            // vertices
            out << "        {\n"
                   "            \"mVertices\": [\n";

            for (size_t vn = 0; vn < mMeshes[i].vertices.size(); vn++)
            {
                out << "                {";
                auto& v = mMeshes[i].vertices[vn];
                out << std::format("\"x\":{:f}, \"y\":{:f}, \"z\":{:f}, \"nx\":{:f}, \"ny\":{:f}, \"nz\":{:f}, \"tx\":{:f}, \"ty\":{:f}, \"tanx\":{:f}, \"tany\":{:f}, \"tanz\":{:f}, \"btanx\":{:f}, \"btany\":{:f}, \"btanz\":{:f}",
                                   v.position[0], v.position[1], v.position[2], 
                                   v.normal[0], v.normal[1], v.normal[2], 
                                   v.uv[0], v.uv[1], 
                                   v.tangent[0], v.tangent[1], v.tangent[2], 
                                   v.bitangent[0], v.bitangent[1], v.bitangent[2]);
                if (vn == mMeshes[i].vertices.size() - 1)
                {
                    out << '}';
                }
                else
                {
                    out << "},\n";
                }
            }
            out << "\n            ],\n";

            // indices
            out << "            \"mIndices\": [\n";
            size_t c = 1;
            for (size_t idx = 0; idx < mMeshes[i].indices.size(); idx++)
            {
                if (c == 1) out << "                ";
                if (idx < mMeshes[i].indices.size() - 1)
                {
                    if ((c % 3) == 0)
                    {
                        out << mMeshes[i].indices[idx] << ",\n";
                        c = 1;
                    }
                    else
                    {
                        out << mMeshes[i].indices[idx] << ',';
                        c++;
                    }
                }
                else
                {
                    out << mMeshes[i].indices[idx];
                    c++;
                }
            }
            out << "\n            ],\n";

            // texture
            out << "            \"mTexture\": {\n";
/*             out << "                \"diffuse\":{\n";
            if (g_meshes[i].diffuse.size())
            {
                out << "                    \"url\":\"" << g_meshes[i].diffuse[0].url << "\",\n";
                out << "                    \"type\":\"" << g_meshes[i].diffuse[0].typeName << "\"";
            }
            out << "\n                },\n";

            out << "                \"specular\":{\n";
            if (g_meshes[i].specular.size())
            {
                out << "                    \"url\":\"" << g_meshes[i].specular[0].url << "\",\n";
                out << "                    \"type\":\"" << g_meshes[i].specular[0].typeName << "\"";
            }
            out << "\n                },\n";

            out << "                \"metallic\":{\n";
            if (g_meshes[i].metallic.size())
            {
                out << "                    \"url\":\"" << g_meshes[i].metallic[0].url << "\",\n";
                out << "                    \"type\":\"" << g_meshes[i].metallic[0].typeName << "\"";
            }
            out << "\n                },\n";

            out << "                \"roughness\":{\n";
            if (g_meshes[i].roughness.size())
            {
                out << "                    \"url\":\"" << g_meshes[i].roughness[0].url << "\",\n";
                out << "                    \"type\":\"" << g_meshes[i].roughness[0].typeName << "\"";
            }
            out << "\n                },\n";

            out << "                \"normal\":{\n";
            if (g_meshes[i].normal.size())
            {
                out << "                    \"url\":\"" << g_meshes[i].normal[0].url << "\",\n";
                out << "                    \"type\":\"" << g_meshes[i].normal[0].typeName << "\"";
            }
            out << "\n                }"; */

            out << "\n            },";
            // texture

            out << "\n            \"mMaterialIndex\": " << mMeshes[i].materialIndex;

            // mesh end
            if (i == mMeshes.size() - 1)
            {
                out << "\n        }";
            }
            else
            {
                out << "\n        },\n";
            }
        }
        //out << "\n    ]\n}";
        out << "\n    ],\n";
        out << "    \"mTextures\": [\n";
        uint32_t matIdx = 0;
        for (const auto& pair : mTextures)
        {
            out << "        \"" << matIdx++ << "\": [\n";
            for (size_t i = 0; i < pair.second.size(); i++)
            {
                out << "{";
                out << "n:" << "\"" << pair.second[i].m_Name << "\",";
                out << "t:" << "\"" << pair.second[i].m_TypeName << "\",";
                out << "},\n";
            }
            out << "        ],\n";
        }
        out << "\n    ]\n}";

        std::ofstream outFile(m_OutPut.c_str());
        if (outFile.is_open())
        {
            outFile << out.rdbuf();
            outFile.close();
        }
    }

    /*void FbxConverter::WriteSkeletonMeshToFile()
    {
        SkeletonMeshNode* pSKMesh = (SkeletonMeshNode*)m_pMeshNode;
        pSKMesh->SetSkeleton(m_pSkeleton);
        std::string& fileSuffix = Resource::GetFileSuffix(Resource::RT_SKELETON_MODEL);
        Stream saveStream;
        saveStream.SetStreamFlag(Stream::AT_REGISTER);
        saveStream.ArchiveAll(m_pMeshNode);
        if (saveStream.NewSave((m_OutPut + m_DefaultName + "." + fileSuffix + ".assert").c_str()))
        {
            FBXSDK_printf("\n\n Write Mesh Done!...\n\n");
        }
    }*/

    /*void FbxConverter::WriteAnimToFile()
    {
        m_pAnimation = new Animation();

        std::vector<BoneKey*> pBoneKeyArray;
        pBoneKeyArray.clear();
        pBoneKeyArray.resize(m_Anim.data.size());
        for (const auto& track : m_Anim.data)
        {
            BoneKey* pBoneKey = new BoneKey;
            pBoneKey->mTranslationArray.clear();
            pBoneKey->mScaleArray.clear();
            pBoneKey->mRotationArray.clear();
            pBoneKey->mBoneName = track.first;

            for (int i = 0; i < track.second.size(); ++i)
            {
                VectorKeyTimeInfo trans;
                trans.m_fKeyTime = track.second[i].keyTime;
                trans.mVector[0] = static_cast<float>(track.second[i].tran[0]);
                trans.mVector[1] = static_cast<float>(track.second[i].tran[1]);
                trans.mVector[2] = static_cast<float>(track.second[i].tran[2]);
                pBoneKey->mTranslationArray.emplace_back(trans);

                VectorKeyTimeInfo scale;
                scale.m_fKeyTime = track.second[i].keyTime;
                scale.mVector[0] = scale.mVector[1] = scale.mVector[2] = 1.0f;
                pBoneKey->mScaleArray.emplace_back(scale);

                QuaternionKeyTimeInfo quaternion;
                quaternion.m_fKeyTime = track.second[i].keyTime;
                quaternion.mQuat.x = static_cast<float>(track.second[i].rot[0]);
                quaternion.mQuat.y = static_cast<float>(track.second[i].rot[1]);
                quaternion.mQuat.z = static_cast<float>(track.second[i].rot[2]);
                quaternion.mQuat.w = static_cast<float>(track.second[i].rot[3]);
                pBoneKey->mRotationArray.emplace_back(quaternion);
            }
            Bone* pBone = m_pSkeleton->GetBone(track.first);
            pBoneKeyArray[pBone->GetIndex()] = pBoneKey;
        }
        m_pAnimation->SetBoneKeyArray(pBoneKeyArray);

        std::string& fileSuffix = Resource::GetFileSuffix(Resource::RT_ANIMATION);
        std::string ResourceName = m_OutPut + m_DefaultName + "." + fileSuffix + ".assert";
        m_pAnimation->SetResourceName(ResourceName);
        Stream saveStream;
        saveStream.SetStreamFlag(Stream::AT_REGISTER);
        saveStream.ArchiveAll(m_pAnimation);
        if (saveStream.NewSave(ResourceName.c_str()))
        {
            FBXSDK_printf("\n\n Write Animation Done!...\n\n");
        }
    }*/

    void FbxConverter::ClearAllInfo()
    {
        mVertexArray.clear();
        mNormalArray.clear();
        for (size_t n = 0; n < 2; n++)
        {
            mTexCoordArray[n].clear();
        }
        mIndexArray.clear();
        mVertexSmGroupArray.clear();
        // mMeshBoneNameArray.clear();
        // mBoneIndexArray.clear();
        // mBoneWeightArray.clear();
    }

    void FbxConverter::GetTriangleMaterialIndex(FbxMesh* pMesh, int triangleCount, std::vector<int>& TriangleMaterialIndexList)
    {
        FbxLayerElementMaterial* pMaterial = pMesh->GetElementMaterial();
        if (!pMaterial)
        {
            return;
        }

        for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
        {
            int materialIndex = pMaterial->GetIndexArray().GetAt(triangleIndex);

            TriangleMaterialIndexList[triangleIndex] = materialIndex;
        }
    }

    void FbxConverter::GetTriangleSmGroupIndex(FbxMesh* pMesh, int triangleCount, std::vector<int>& TriangleSmGroupIndexList)
    {
        FbxLayerElementSmoothing* pSmoothing = pMesh->GetElementSmoothing();
        if (pSmoothing)
        {
            bool bDirectSm = (pSmoothing->GetReferenceMode() == FbxLayerElement::eDirect);

            for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
            {
                int SmIndex    = bDirectSm ? triangleIndex : pSmoothing->GetIndexArray().GetAt(triangleIndex);
                int iSmoothing = pSmoothing->GetDirectArray().GetAt(SmIndex);

                TriangleSmGroupIndexList[triangleIndex] = iSmoothing;
            }
        }
    }

    void FbxConverter::ReadVertex(FbxMesh* pMesh, int ctrlPointIndex, FbxVector4& V)
    {
        FbxVector4* ctrlPoints = pMesh->GetControlPoints();
        V                      = ctrlPoints[ctrlPointIndex];
    }

    void FbxConverter::ReadNormal(FbxMesh* pMesh, int ctrlPointIndex, int vertexCounter, FbxVector4& N)
    {
        if (pMesh->GetElementNormalCount() < 1)
        {
            return;
        }

        FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(0);
        switch (leNormal->GetMappingMode())
        {
            case FbxGeometryElement::eByControlPoint:
            {
                switch (leNormal->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                    {
                        N = leNormal->GetDirectArray().GetAt(ctrlPointIndex);
                    }
                    break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leNormal->GetIndexArray().GetAt(ctrlPointIndex);
                        N      = leNormal->GetDirectArray().GetAt(id);
                    }
                    break;
                    default:
                        break;
                }
            }
            break;
            case FbxGeometryElement::eByPolygonVertex:
            {
                switch (leNormal->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                    {
                        N = leNormal->GetDirectArray().GetAt(vertexCounter);
                    }
                    break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leNormal->GetIndexArray().GetAt(vertexCounter);
                        N      = leNormal->GetDirectArray().GetAt(id);
                    }
                    break;
                    default:
                        break;
                }
                break;
            }
        }
    }

    void FbxConverter::ReadUV(FbxMesh* pMesh, int ctrlPointIndex, int triangleIndex, int triangleVertexIndex, int uvIndex, FbxVector2& UV)
    {
        FbxGeometryElementUV* pUV = pMesh->GetElementUV(uvIndex);
        switch (pUV->GetMappingMode())
        {
            case FbxGeometryElement::eByControlPoint:
            {
                switch (pUV->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                        UV = pUV->GetDirectArray().GetAt(ctrlPointIndex);
                        // FBXSDK_printf("eByControlPoint UV eDirect u=%f, v=%f", vertex.UVs[0], vertex.UVs[1]);
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = pUV->GetIndexArray().GetAt(ctrlPointIndex);
                        UV     = pUV->GetDirectArray().GetAt(id);
                        // FBXSDK_printf("eByControlPoint UV eIndexToDirect u=%f, v=%f", vertex.UVs[0], vertex.UVs[1]);
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                }
            }
            break;

            case FbxGeometryElement::eByPolygonVertex:
            {
                int textureUVIndex = pMesh->GetTextureUVIndex(triangleIndex, triangleVertexIndex);
                switch (pUV->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        UV = pUV->GetDirectArray().GetAt(textureUVIndex);
                        // FBXSDK_printf("eByPolygonVertex UV eIndexToDirect u=%f, v=%f", vertex.UVs[0], vertex.UVs[1]);
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                }
            }
            break;

            default:
                break;
        }
    }

    void FbxConverter::CreateStaticMesh(const std::string& name, int UVNum, bool hasSkin)
    {
        if (mVertexArray.size() == 0)
        {
            return;
        }
        //ASSERT(result.second);

        Mesh mesh {};
        mesh.vertices.reserve(mVertexArray.size());
        for (size_t i = 0; i < mVertexArray.size(); i++)
        {
            Vertex v {};
            v.position = mVertexArray[i];
            v.normal   = mNormalArray[i];
            if (UVNum > 0)
            {
                v.uv = mTexCoordArray[0][i];
            }
            mesh.vertices.emplace_back(v);
        }
        mesh.indices       = mIndexArray;
        mesh.materialIndex = mMaterialCount;
        mMeshes.emplace_back(mesh);

        /* MeshData tmp;
        auto result = mMeshes.insert(std::make_pair(name, std::move(tmp)));
        ASSERT(result.second);

        MeshData& mesh = result.first->second;
        mesh.mVertices.resize(mVertexArray.size());
        for (size_t i = 0; i < mVertexArray.size(); i++)
        {
            // ����
            mesh.mVertices[i].x = static_cast<float>(mVertexArray[i][0]);
            mesh.mVertices[i].y = static_cast<float>(mVertexArray[i][1]);
            mesh.mVertices[i].z = static_cast<float>(mVertexArray[i][2]);
            // ����
            mesh.mVertices[i].nx = static_cast<float>(mNormalArray[i][0]);
            mesh.mVertices[i].ny = static_cast<float>(mNormalArray[i][1]);
            mesh.mVertices[i].nz = static_cast<float>(mNormalArray[i][2]);
            // UV
            if (UVNum > 0)
            {
                mesh.mVertices[i].tx = static_cast<float>(mTexCoordArray[0][i][0]);
                mesh.mVertices[i].ty = static_cast<float>(mTexCoordArray[0][i][1]);
            }
        } */

        /*
        //Ȩ��
        DataBuffer* pWeightData = nullptr;
        if (hasSkin)
        {
            pWeightData = new DataBuffer;
            if (!pWeightData)
                return;
            pWeightData->SetData(&mBoneWeightArray[0][0], (unsigned int)mBoneWeightArray.size(), ShaderDataType::Float4);
        }
        //��������
        DataBuffer* pBoneIndexData = nullptr;
        if (hasSkin)
        {
            pBoneIndexData = new DataBuffer;
            if (!pBoneIndexData)
                return;

            std::vector<glm::ivec4> IndexArray;
            IndexArray.clear();
            for (size_t i = 0; i < mBoneIndexArray.size(); i++)
            {
                glm::ivec4 v;
                for (int j = 0; j < 4; j++)
                {
                    if (mBoneIndexArray[i][j] >= 0)
                    {
                        v[j] = m_pSkeleton->GetBone(mMeshBoneNameArray[mBoneIndexArray[i][j]])->GetIndex();
                    }
                    else
                    {
                        v[j] = 0;
                    }
                }
                IndexArray.emplace_back(v);
            }
            pBoneIndexData->SetData(&IndexArray[0][0], (unsigned int)IndexArray.size(), ShaderDataType::Int4);
        }
        */

        // ����
        //mesh.mIndices = mIndexArray;

        /*
        //VertexBuffer
        VertexBuffer* pVB = new VertexBuffer;
        pVB->SetData(VertexFormat::VF_POSITION, pVertexData);
        pVB->SetData(VertexFormat::VF_NORMAL, pNormalData);
        for (int i = 0; i < UVNum && i < 2; i++)
        {
            pVB->SetData(VertexFormat::VF_TEXCOORD, pTexCoordData[i]);
        }
        if (hasSkin)
        {
            pVB->SetData(VertexFormat::VF_BLENDINDICES, pBoneIndexData);
            pVB->SetData(VertexFormat::VF_BLENDWEIGHT, pWeightData);
        }

        //BufferLayout
        BufferLayout layout;
        LayoutElement LE;
        LE.m_Type = ShaderDataType::Float3;
        LE.m_uiSize = ShaderDataTypeSize(LE.m_Type);
        LE.m_uiOffset = 0;
        LE.m_Semantics = VertexFormat::VF_POSITION;
        LE.m_SemanticsIndex = 0;
        LE.m_bNormalize = false;
        layout.AddElement(LE);

        LE.m_Type = ShaderDataType::Float3;
        LE.m_uiSize = ShaderDataTypeSize(LE.m_Type);
        LE.m_uiOffset = ShaderDataTypeSize(ShaderDataType::Float3);
        LE.m_Semantics = VertexFormat::VF_NORMAL;
        LE.m_SemanticsIndex = 0;
        LE.m_bNormalize = false;
        layout.AddElement(LE);

        unsigned int UVOffset = ShaderDataTypeSize(ShaderDataType::Float3) + ShaderDataTypeSize(ShaderDataType::Float3);
        for (int i = 0; i < UVNum && i < 2; i++)
        {
            LE.m_Type = ShaderDataType::Float2;
            LE.m_uiSize = ShaderDataTypeSize(LE.m_Type);
            LE.m_uiOffset = UVOffset;
            LE.m_Semantics = VertexFormat::VF_TEXCOORD;
            LE.m_SemanticsIndex = i;
            LE.m_bNormalize = false;
            layout.AddElement(LE);
            UVOffset += ShaderDataTypeSize(ShaderDataType::Float2);
        }
        if (hasSkin)
        {
            LE.m_Type = ShaderDataType::Float4;
            LE.m_uiSize = ShaderDataTypeSize(LE.m_Type);
            LE.m_uiOffset = UVOffset;
            LE.m_Semantics = VertexFormat::VF_BLENDWEIGHT;
            LE.m_SemanticsIndex = 0;
            LE.m_bNormalize = false;
            layout.AddElement(LE);

            LE.m_Type = ShaderDataType::Int4;
            LE.m_uiSize = ShaderDataTypeSize(LE.m_Type);
            LE.m_uiOffset = UVOffset + ShaderDataTypeSize(ShaderDataType::Float4);
            LE.m_Semantics = VertexFormat::VF_BLENDINDICES;
            LE.m_SemanticsIndex = 0;
            LE.m_bNormalize = false;
            layout.AddElement(LE);
        }

        pVB->SetLayout(layout);

        //IndexBuffer
        IndexBuffer* pIB = new IndexBuffer(pIndexData);

        Mesh* pMesh = new Mesh;
        pMesh->SetVertexBuffer(pVB);
        pMesh->SetIndexBuffer(pIB);

        Geometry* pGeo = new Geometry;
        pGeo->SetMeshData(pMesh);
        m_pGeoNode->AddChild(pGeo);*/
    }

} // namespace FakeReal