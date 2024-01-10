//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05
class CShader;
class CFbxModelObject;

struct CB_GAMEOBJECT_INFO
{
	Matrix4x4					worldMatrix;
	UINT						m_nMaterial;
};

struct MATERIAL
{
	Vector4						m_xmf4Ambient;
	Vector4						m_xmf4Diffuse;
	Vector4						m_xmf4Specular; //(r,g,b,a=power)
	Vector4						m_xmf4Emissive;
};


struct CB_BONE_TRANSFORMS
{
	XMFLOAT4X4 BoneTransforms[SKINNED_ANIMATION_BONES];
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	Vector4						m_xmf4Albedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	UINT							m_nReflection = 0;
	CTexture						*m_pTexture = NULL;
	CShader							*m_pShader = NULL;

	void SetAlbedo(Vector4 xmf4Albedo) { m_xmf4Albedo = xmf4Albedo; }
	void SetReflection(UINT nReflection) { m_nReflection = nReflection; }
	void SetTexture(CTexture *pTexture);
	void SetShader(CShader *pShader);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct BoneInfo
{
	string					boneName;
	Matrix4x4				boneOffset;
	Matrix4x4				finalTransform;
};

////ModelFrameData
//struct asKeyFramedata
//{
//	//몇 프레임 인지
//	float					Time = 0.0f;
//
//	//해당 본이 어느정도 움직일지에 대한 SRT
//	Vector3					Scale = Vector3::one;
//	Quaternion				Rotation = Quaternion::identity;
//	Vector3					Translation = Vector3::one;
//
//	//Matrix4x4 finalTransform = Matrix4x4::identity;
//};

struct asKeyFramedataQuaternion
{
	//몇 프레임 인지
	float					Time = 0.0f;

	//해당 본이 어느정도 움직일지에 대한 R
	Quaternion				value = Quaternion::identity;
};

struct asKeyFramedataVector3
{
	//몇 프레임 인지
	float					Time = 0.0f;

	//해당 본이 어느정도 움직일지에 대한 T
	Vector3					value = Vector3::one;
};
//modelKeyFrame

struct asClipNode
{
	std::string								clipNodeName;
	vector<asKeyFramedataVector3>			KeyframeScale;
	vector<asKeyFramedataQuaternion>		KeyframeRotation;
	vector<asKeyFramedataVector3>			KeyframeTranslation;
};

//ModelClip
struct asClip
{
	string					clipName;

	UINT					FrameCount;	//클립의 전체 프레임 수	
	float					Duration;		//클립(애니메이션)의 길이

	vector<asClipNode*>		keyFrames;		//키프레임 정보
	//std::map < std::string, asClipNode*> keyFrames;
};

class asAnimationTrack
{
public:
	asAnimationTrack() {}
	~asAnimationTrack() {}

public:
	//사용할것인지?
	//기본은 false, 재생할 애니메이션만 true로
	bool trackEnable = false;

	//재생 속도
	float animationSpeed = 1.0f;
	//클립의 가중치
	float animationWeight = 1.0f;
	//재생을 시작할 부분
	float timeStartPosition = 0.0f;
	//재생을 끝낼 부분 = duration
	float timeEndPosition = 0.0f;


	//거짓이면 정방향 실행중, 참이면 역방향 실행중
	bool animationDurationReverse = false;

	//재생 방식
	ANIMATION_PLAY_TYPE animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;

	//클립의 번호
	int clipNumber = 0;

};

//AnimationController
class ModelAnimator
{
public:
	ModelAnimator( const aiScene* scene, CFbxModelObject* fbxModelObejct = NULL, const char* filename = NULL);
	~ModelAnimator();

	void Update(int animationNum, std::chrono::system_clock::time_point animationTime);

	void SetAnimationClip(const aiScene* scene);
public:

	//키프레임 데이터 읽어오는 곳
	void ExtractBoneTransforms(float animationTime, const int animationIndex, asAnimationTrack* track);

	void ReadNodeHierarchy(float animationTime, int animationClipNumber, asAnimationTrack* track, CFbxModelObject* node, const Matrix4x4& parentTransform);
	void SetBoneTransform(CFbxModelObject* node, vector<BoneInfo> rootbone, asAnimationTrack* track);

	asClipNode* FindNodeAnim(asClip* animation, const string nodeName);
	Vector3 CalcInterpolatedVectorFromKey(float animationTime, const int numKeys, std::vector<asKeyFramedataVector3> keyFrameData);
	Quaternion CalcInterpolatedQuaternionFromKey(float animationTime, const int numKeys, std::vector<asKeyFramedataQuaternion> keyFrameData);

	void SaveAnimationTransform();

public:
	//애니메이션 정보들(이름 등등)
	//1개당 1동작(걷기, 뛰기, 달리기 등등)
	std::vector<asClip*> animationClips;

	//애니메이션 재생 트랙	(앞으로 모든 애니메이션 속도, 시작시간 등은 여기서 관리)
	std::vector<asAnimationTrack*> animationTrack;

	CFbxModelObject* rootNode = NULL;
	//UINT m_count = 0;	//애니메이션 개수
	
	int currentPlayAnimationNum = 0;
	std::chrono::system_clock::time_point currentPlayAnimationStartTime;

	bool isPlay = true;

	bool aniOncePlay = false;
	bool aniOnceIsPlaying = false;

	bool isCurrentAniAttack = false;

public:
	bool isAniOncePlayFinish() { return aniOncePlay; }

	void animationOutputFromFile();
	bool outputCheck = true;	//참이면 아직 파일 안만든것.
	std::string modelName;

	std::vector<Matrix4x4>			addBoneTransforms;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
public:
	CGameObject(int nMeshes=1);
    virtual ~CGameObject();

	void SetMesh(int nIndex, CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetMaterial(CMaterial *pMaterial);
	void SetName(const char* name);

	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { cbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { cbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(cbvGPUDescriptorHandle); }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Update(float fTimeElapsed, CCamera* camera) {}
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances);
	virtual void ReleaseUploadBuffers();

	Vector3 GetPosition();
	Vector3 GetLook();
	Vector3 GetUp();
	Vector3 GetRight();

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(Vector3 xmf3Position);

	virtual void SetScale(float x, float y, float z);

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(Vector3* pxmf3Axis, float fAngle);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	bool ColideObjectByRayIntersection(Vector3& position, Vector3& direction, float* distance);
	void PrintBoundingBox();
	virtual void SetHp(UINT damage) {};

	void SetWorldMatrix(Matrix4x4 world) { worldMatrix = world; }
	Matrix4x4 GetWorldMatrix() { return worldMatrix; }

	void SetName(string name) { objectName = name; }
	string GetName() { return objectName; }

	OBJECT_TYPE GetType() { return objectType; }

	void SetIsAlive(bool alive) { isAlive = alive; }
	bool GetIsAlive() { return isAlive; }

	bool IsInFrustum(CCamera* camera);

	BoundingOrientedBox				objectBoundingBox;

protected:
	Matrix4x4						worldMatrix;
	D3D12_GPU_DESCRIPTOR_HANDLE		cbvGPUDescriptorHandle;
	CMaterial*						objectMaterial = NULL;
	CMesh**							meshes;
	int								numMeshes;
	ID3D12Resource*					cbGameObject = NULL;
	CB_GAMEOBJECT_INFO*				cbMappedGameObject = NULL;
	map< string, TEXTURE_INFO>		textures;

	string							objectName = "NoName";

	OBJECT_TYPE						objectType = OBJECT_TYPE::STATIC;
	vector<OBJECT_STATE>			objectStates;

	UINT nMat = 1;
	bool							isAlive = false;
	bool							Dying = true;
};	


class CFbxModelObject : public CGameObject
{
public:
	CFbxModelObject(int nMeshes = 1);
	~CFbxModelObject();

	void SetChild(CFbxModelObject* child, int index);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseUploadBuffers();

	void		 UpdateTransform(Matrix4x4* parent);
	virtual void SetWorld(Matrix4x4 World);
	virtual void SetPosition(const Vector3& xmf3Position);

	virtual void SetScale(float x, float y, float z);
	virtual void SetScaleInverse(float x, float y, float z);

	virtual void SetDirectionWithLookVector(Vector3 Look);
	virtual void SetDirectionWithPlayerLookVector(Vector3 Look);
	virtual void SetDirectionWithMinoLookVector(Vector3 Look);

	virtual void Move(Vector3 pos);
	virtual void Move(float x, float y, float z);

	virtual void Rotate(Vector3* pxmf3Axis, float fAngle);
	virtual void Rotate(float x, float y, float z);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void SetSsaoMaterial(UINT n);

	virtual void Update();
	void DebugWorldMatrix();

	static CFbxModelObject* LoadFbxRootObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, bool ccw, std::map< string, TEXTURE_INFO> textures,CShader* shader, const aiScene* scene, bool isSkin = true, const char* filename = NULL);
	static CFbxModelObject* LoadObjectHierarchy(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, aiNode* node, const aiScene* scene, std::map< string, TEXTURE_INFO> textures, CShader* shader, int* objectNum, bool isRoot, const char* filename, bool isSkin = true, CFbxModelObject* parent = NULL, CFbxModelObject* root = NULL);
	vector < CFbxModelObject*>		m_Child;
	CFbxModelObject*				m_Parent = NULL;
	CFbxModelObject*				m_RootObject = NULL;
	Matrix4x4						m_Transform;
	Matrix4x4						m_nodeTransform;
	Matrix4x4*						m_Offsets = NULL;
	Matrix4x4						m_RotationMatrix = Matrix4x4::identity;

	std::vector<BoneInfo>			boneInfos;
	std::vector<BoneInfo>			rootBoneInfos;
	ID3D12Resource*					BoneTransforms = NULL;
	CB_BONE_TRANSFORMS*				MappedBoneTransforms = NULL;

	bool isRoot = false;

public:
	ModelAnimator* m_pModelAnimator = NULL;

	//피격 애니메이션 번호 저장용
	//나중에 다른 애니메이션 저장도 편리하게 하기 위해서 맵으로 만들 예정
	int hitAniNum = 0;
	int OpenAniNum = 0;
	int CloseAniNum = 0;
	bool is_open = false;

	int currentAniNum = 0;
	int pastAniNum = 0;
	bool isAniChange = false;
	std::chrono::system_clock::time_point currentAniTime;

	void AnimationChange(int num);

	void IsHit();
	void IsOpen();
	//쉐이더 상수 버퍼에 올려줄 SRT 결과 담는 곳
	std::vector<Matrix4x4>			transforms;

	std::map<std::string, int> animationManual;

};

class CTestWallObject : public CGameObject
{
public:
	CTestWallObject(int nMeshes = 1);
	virtual ~CTestWallObject();


public:
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, Vector3 xmf3Scale);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage*				m_pHeightMapImage;

	int								m_nWidth;
	int								m_nLength;

	Vector3							m_xmf3Scale;

public:
	float GetHeight(float x, float z, bool bReverseQuad = false) { return 1000; } //World
	Vector3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	Vector3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};


class CTerrainLava : public CGameObject
{
public:
	CTerrainLava(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,float width, float height, int m, int n, CShader* shader);
	virtual ~CTerrainLava();
};
