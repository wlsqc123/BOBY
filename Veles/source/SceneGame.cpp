//-----------------------------------------------------------------------------
// File: SceneGame.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "SceneGame.h"
#include <random>
SceneGame::SceneGame() : Scene()
{
	gsShadowSrvDescHeap = NULL;
	gsFogPos = 300;
}

SceneGame::~SceneGame()
{
}

void SceneGame::Init(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)	//���⼭ ������Ʈ�� ���������.
{
	snd = new Fmod_snd();
	gsDrawfog = false;
	deltaTime = std::chrono::system_clock::now();
	gsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateShadowMap(pd3dDevice);
	gsSsao = std::make_unique<Ssao>(pd3dDevice, pd3dCommandList, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	CreateShadowDescriptorHeap(pd3dDevice);

	for (UINT i = 0; i < MAX_SHADOWMAP; i++)
		gsShadowMaps[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	gsSsao->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	skinShader = new CSkinnedAnimationShader;
	skinShader->CreateShader(pd3dDevice, gsRootSignature);
	CShader* normalMapShader = new CMonsterShader;
	normalMapShader->CreateShader(pd3dDevice, gsRootSignature);
	CShader* lavaShader = new LavaShader;
	lavaShader->CreateShader(pd3dDevice, gsRootSignature);
	CShader* uiShader = new CUiShader;
	uiShader-> CreateShader(pd3dDevice, gsRootSignature);
	CShader* effectShader = new CEffectShader;
	effectShader->CreateShader(pd3dDevice, gsRootSignature);
	CShader* fireShader = new CFireEffectShader;
	fireShader->CreateShader(pd3dDevice, gsRootSignature);
	system("cls");
	cout << "Shader Compile" << endl;

	gsSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, gsRootSignature);	
	gsTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, gsRootSignature, _T("Image/map1-4.raw"), 513, 513, 513, 513, Vector3(10.0f, 25.0f, 10.0f));

	gsShotgunPlayer = new CShotgunPlayer(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, 1, skinShader, uiShader);
	gsRiflePlayer = new CRiflePlayer(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, 1, skinShader, uiShader);
	gsSniperPlayer = new CSniperPlayer(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, 1, skinShader, uiShader);

	gsLavaMgr = std::make_unique<LavaWave>(pd3dDevice, pd3dCommandList, 256, 256, 1.0f, 0.03f, 2.0f, 0.5f);
	gsLavaInfo = new LAVAINFO;
	gsLavaInfo->gDisplacementMapTexelSize.x = 1.0f / gsLavaMgr->ColumnCount();
	gsLavaInfo->gDisplacementMapTexelSize.x = 1.0f / gsLavaMgr->RowCount();
	gsLavaInfo->gGridSpatialStep = gsLavaMgr->SpatialStep();
	gsLavaInfo->gTextureAnimation = Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	CTerrainLava* lavaObject = new CTerrainLava(pd3dDevice, pd3dCommandList, gsRootSignature, 1500, 1500, gsLavaMgr->RowCount(), gsLavaMgr->ColumnCount(), lavaShader);
	lavaObject->SetPosition(Vector3(665.265, 1260, 3328.36));
	gsLavaObject.push_back(lavaObject);

	gsOtherPlayer.reserve(MAX_PLAYER - 1);
	gsMonster.reserve(MAX_OBJECT);
	
	Assimp::Importer importer;
	std::string error;
	const aiScene* scene;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	system("cls");
	cout << "add Player" << endl;
	scene = importer.ReadFile("Models/soldier.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!scene) { error = (importer.GetErrorString()); }
	for (int i = 0; i < MAX_PLAYER - 1; i++)
	{
		CTestPlayerModel* player = new CTestPlayerModel(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
		gsOtherPlayer.push_back(player);
	}
	system("cls");
	cout << "add Monster 0%" << endl;
	scene = importer.ReadFile("Models/magma.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_OptimizeMeshes);
	if (!scene) { error = (importer.GetErrorString()); }
	for (int i = 0; i < MAGMAMONSTER_NUM; i++)
	{
		CMovableHierarchyObject* monsterObj = new CMagmaMonster(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
		monsterObj->SetObjectID(i);
		monsterObj->SetName("Magma " + to_string(i));
		gsMonster.push_back(monsterObj);
	}
	system("cls");
	cout << "add Monster 20%" << endl;
	scene = importer.ReadFile("Models/fireGolem_maya.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded| aiProcess_OptimizeMeshes);
	if (!scene) { error = (importer.GetErrorString()); }
	for (int i = 0; i < GOLEMMONSTER_NUM; i++)
	{
		CMovableHierarchyObject* monsterObj = new CGolemModel(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
		monsterObj->SetObjectID(i + MAGMAMONSTER_NUM);
		monsterObj->SetName("Golem " + to_string(i));
		gsMonster.push_back(monsterObj);
	}
	system("cls");
	cout << "add Monster 40%" << endl;
	scene = importer.ReadFile("Models/golem.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FlipWindingOrder);
	if (!scene) { error = (importer.GetErrorString()); }
	for (int i = 0; i < ORGEMONSTER_NUM; i++)
	{
		CMovableHierarchyObject* monsterObj = new COGREMonster(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
		monsterObj->SetObjectID(i + MAGMAMONSTER_NUM + GOLEMMONSTER_NUM);
		monsterObj->SetName("Ogre " + to_string(i));
		gsMonster.push_back(monsterObj);
	}
	system("cls");
	cout << "add Monster 70%" << endl;
	scene = importer.ReadFile("Models/Chest.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_OptimizeMeshes);
	if (!scene) { error = (importer.GetErrorString()); }
	for (int i = 0; i < CHESTOBJECT_NUM; i++)
	{
		ChestObject* chestObj = new ChestObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene , skinShader);
		chestObj->SetObjectID(i);
		chestObj->SetName("Chest " + to_string(i));
		gsInteractObjects.push_back(chestObj);
	}
	gsInteractObjects[2]->Rotate(&Vector3(0, 1, 0), 180);
	gsInteractObjects[3]->Rotate(&Vector3(0, 1, 0), 180);
	gsInteractObjects[4]->Rotate(&Vector3(0, 1, 0), 180);
	system("cls");
	cout << "add Monster 100%" << endl;
	scene = importer.ReadFile("Models/anim_door.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_OptimizeMeshes);
	if (!scene) { error = (importer.GetErrorString()); }
	//이부분 위치 서버에서 넘겨서 받아야할거임
	short i = 0;
	DoorObject* doortObj = new DoorObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
	doortObj->SetObjectID(i);
	doortObj->SetName("door " + to_string(i++));
	doortObj->Rotate(&Vector3(0, 1, 0), -90);
	//doortObj->SetPosition(Vector3(780, 420, 1500));
	gsInteractObjects.push_back(doortObj);

	doortObj = new DoorObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
	doortObj->SetObjectID(i);
	doortObj->Rotate(&Vector3(0, 1, 0), 90);
	doortObj->SetName("door " + to_string(i++));
	//doortObj->SetPosition(Vector3(780, 420, 2670));
	gsInteractObjects.push_back(doortObj);

	doortObj = new DoorObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
	doortObj->SetObjectID(i);
	doortObj->Rotate(&Vector3(0, 1, 0), -90);
	doortObj->SetName("door " + to_string(i++));
	//doortObj->SetPosition(Vector3(1500, 420, 3280));
	gsInteractObjects.push_back(doortObj);

	doortObj = new DoorObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
	doortObj->SetObjectID(i);
	doortObj->SetName("door " + to_string(i++));
	//doortObj->Rotate(&Vector3(0, 1, 0), 90);
	//doortObj->SetPosition(Vector3(2665, 420, 3280));
	gsInteractObjects.push_back(doortObj);

	doortObj = new DoorObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
	doortObj->SetObjectID(i);
	doortObj->SetName("door " + to_string(i++));
	doortObj->Rotate(&Vector3(0, 1, 0), 180);
	//doortObj->SetPosition(Vector3(3280, 420, 2680));
	gsInteractObjects.push_back(doortObj);

	doortObj = new DoorObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
	doortObj->SetObjectID(i);
	doortObj->SetName("door " + to_string(i++));
	//doortObj->SetPosition(Vector3(2040, 930, 1500));
	gsInteractObjects.push_back(doortObj);

	scene = importer.ReadFile("Models/lever.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_OptimizeMeshes);
	if (!scene) { error = (importer.GetErrorString()); }
	LeverObject* leverObj =  new LeverObject(pd3dDevice, pd3dCommandList, gsRootSignature, snd, gsTerrain, scene, skinShader);
	leverObj->SetObjectID(0);
	leverObj->SetName("lever" + to_string(0));
	leverObj->Rotate(&Vector3(0, 1, 0), -90);
	//leverObj->SetPosition(Vector3(1630, 540, 2680));
	gsInteractObjects.push_back(leverObj);

	scene = importer.ReadFile("Models/railing.FBX", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_OptimizeMeshes);
	if (!scene) { error = (importer.GetErrorString()); }
	StairObject* mudObj = new StairObject(pd3dDevice, pd3dCommandList, gsRootSignature, gsTerrain, scene, skinShader);
	mudObj->SetObjectID(0);
	mudObj->SetName("Stair " + to_string(0));
	gsInteractObjects.push_back(mudObj);

	scene = importer.ReadFile("Models/Rock5_maya.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded );
	if (!scene) { error = (importer.GetErrorString()); }
	for (int i = 0; i < 5; ++i)
	{
		Stone5Object* stone5Obj1 = new Stone5Object(pd3dDevice, pd3dCommandList, gsRootSignature, gsTerrain, scene, normalMapShader);
		gsStones.push_back(stone5Obj1);
	}
	scene = importer.ReadFile("Models/Rock6_maya.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded );
	if (!scene) { error = (importer.GetErrorString()); }

	for (int i = 0; i < 5; ++i)
	{
		Stone6Object* stone6Obj = new Stone6Object(pd3dDevice, pd3dCommandList, gsRootSignature, gsTerrain, scene, normalMapShader);
		gsStones.push_back(stone6Obj);
	}


	InstancingShader* pObjectShader = new InstancingShader();
	pObjectShader->CreateShader(pd3dDevice, gsRootSignature);
	pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, gsTerrain);
	gsMapObject.push_back(pObjectShader);
	pObjectShader = new InstancingRowShader();
	pObjectShader->CreateShader(pd3dDevice, gsRootSignature);
	pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, gsTerrain);
	gsMapObject.push_back(pObjectShader);

	EffectGunFire* gfEffect = new EffectGunFire(pd3dDevice, pd3dCommandList, gsRootSignature, effectShader);
	gsEffects.push_back(gfEffect);
	EffectCollision* colEffect;
	for (int i = 0; i < 5; ++i)
	{
		colEffect = new EffectCollision(pd3dDevice, pd3dCommandList, gsRootSignature,fireShader);
		gsEffects.push_back(colEffect);
	}
	EffectBlood* bloodEffect;
	for (int i = 0; i < 5; ++i)
	{
		bloodEffect = new EffectBlood(pd3dDevice, pd3dCommandList, gsRootSignature, effectShader);
		gsEffects.push_back(bloodEffect);
	}
	UITexture* crossHair = new UITexture(pd3dDevice, pd3dCommandList, gsRootSignature, L"Image/crosshair2.dds", Vector2(0, 0), Vector2(0.05f, 0.07f),Vector2(1,1), uiShader);
	gsEffects.push_back(crossHair);

	EffectDecal* decal;
	gsEffectsDecal.reserve(60);
	for (int i = 0; i < 60; ++i)
	{
		decal = new EffectDecal(pd3dDevice, pd3dCommandList, gsRootSignature, effectShader);
		gsEffectsDecal.push_back(decal);
	}
	EffectRangeAttack* ra;
	gsEffectRangeAttack.reserve(20);
	for (int i = 0; i < 20; ++i)
	{
		ra = new EffectRangeAttack(pd3dDevice, pd3dCommandList, gsRootSignature, effectShader);
		gsEffectRangeAttack.push_back(ra);
	}

	ZONE zone;
	//시작방
	zone.monsterID = { 3,4,10,11 };
	gsZone.push_back(zone);
	//중앙방
	zone.monsterID = { 0,1,5,6,7,12,13,14 };
	gsZone.push_back(zone);
	zone.monsterID = { 2, 15 ,16 };
	gsZone.push_back(zone);
	//안개방?
	zone.monsterID = { 8 };
	gsZone.push_back(zone);
	//보스방
	zone.monsterID = { 9 };
	gsZone.push_back(zone);


	BuildLightsAndMaterials();
	gsLights->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	for (int i = 0; i < MAX_PLAYER - 1; ++i)
		gsLights->setLightAlive(i + 2, true);

	startTime = currentTime = deltaTime = std::chrono::system_clock::now();
	gsItemPacket.doSend = false;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	cout << "ADD OBJECT COMPLETE" << endl;
}

ID3D12RootSignature *SceneGame::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[7];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2; //GameObject
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 10; 
	pd3dDescriptorRanges[1].BaseShaderRegister = 14; //t14: gtxtTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 3;
	pd3dDescriptorRanges[2].BaseShaderRegister = 5; //t5: gtxtTerrain
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 8; //t8: gtxtSkyBoxTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = MAX_SHADOWMAP;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: shadowmaps
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 9; //t9: ssaoMap
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 4; //t9: ssaoMap
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[18];

	pd3dRootParameters[PLAYER_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[PLAYER_CBV].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[PLAYER_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[PLAYER_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[CAMERA_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[CAMERA_CBV].Descriptor.ShaderRegister = 1; 
	pd3dRootParameters[CAMERA_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[CAMERA_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[OBJECT_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[OBJECT_CBV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[OBJECT_CBV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[OBJECT_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	pd3dRootParameters[MATERIAL_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[MATERIAL_CBV].Descriptor.ShaderRegister = 4;
	pd3dRootParameters[MATERIAL_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[MATERIAL_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[LIGHT_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[LIGHT_CBV].Descriptor.ShaderRegister = 5;
	pd3dRootParameters[LIGHT_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[LIGHT_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[TEXTURE_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[TEXTURE_SRV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[TEXTURE_SRV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; 
	pd3dRootParameters[TEXTURE_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[TERRAIN_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[TERRAIN_SRV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[TERRAIN_SRV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];
	pd3dRootParameters[TERRAIN_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[SKYBOX_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[SKYBOX_SRV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[SKYBOX_SRV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[SKYBOX_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[SHADOWMAP_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[SHADOWMAP_SRV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[SHADOWMAP_SRV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
	pd3dRootParameters[SHADOWMAP_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[SHADOWMAP_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[SHADOWMAP_CBV].Descriptor.ShaderRegister = 6;
	pd3dRootParameters[SHADOWMAP_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[SHADOWMAP_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[SKINNEDBONETRANSFORMS_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[SKINNEDBONETRANSFORMS_CBV].Descriptor.ShaderRegister = 7;
	pd3dRootParameters[SKINNEDBONETRANSFORMS_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[SKINNEDBONETRANSFORMS_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[EFFECT_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[EFFECT_CBV].Descriptor.ShaderRegister = 8;
	pd3dRootParameters[EFFECT_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[EFFECT_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 9;
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[SHADOWMAPS_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[SHADOWMAPS_CBV].Descriptor.ShaderRegister = 10;
	pd3dRootParameters[SHADOWMAPS_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[SHADOWMAPS_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[SSAOMAP_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[SSAOMAP_SRV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[SSAOMAP_SRV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5];
	pd3dRootParameters[SSAOMAP_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[LAVAWAVE_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[LAVAWAVE_CBV].Descriptor.ShaderRegister = 11;
	pd3dRootParameters[LAVAWAVE_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[LAVAWAVE_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[LAVAWAVE_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[LAVAWAVE_SRV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[LAVAWAVE_SRV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[6];
	pd3dRootParameters[LAVAWAVE_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[MAP_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[MAP_SRV].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[MAP_SRV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[MAP_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[5];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 16;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[3].Filter = D3D12_FILTER_ANISOTROPIC;
	pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[3].MipLODBias = 0;
	pd3dSamplerDescs[3].MaxAnisotropy = 8;
	pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[3].MinLOD = 0;
	pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pd3dSamplerDescs[3].ShaderRegister = 3;
	pd3dSamplerDescs[3].RegisterSpace = 0;
	pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[4].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	pd3dSamplerDescs[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[4].MipLODBias = 0;
	pd3dSamplerDescs[4].MaxAnisotropy = 8;
	pd3dSamplerDescs[4].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[4].MinLOD = 0;
	pd3dSamplerDescs[4].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[4].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pd3dSamplerDescs[4].ShaderRegister = 4;
	pd3dSamplerDescs[4].RegisterSpace = 0;
	pd3dSamplerDescs[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void SceneGame::BuildLightsAndMaterials()
{
	//���� �־��ݴϴ�~.
	LIGHTS *pLights = new LIGHTS;
	::ZeroMemory(pLights, sizeof(LIGHTS));
	bossStagePointRange = 300.f;
	pLights->m_xmf4GlobalAmbient = Vector4(0.8f, 0.8f, 0.8f, 1.0f);

	pLights->m_pLights[0].m_bEnable =true;
	pLights->m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	pLights->m_pLights[0].m_xmf4Ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	pLights->m_pLights[0].m_xmf4Diffuse = Vector4(0.3f, 0.3f, 0.3f, 1.0f);
	pLights->m_pLights[0].m_xmf4Specular = Vector4(0.1f, 0.1f, 0.1f, 0.0f);
	pLights->m_pLights[0].m_xmf3Direction = Vector3(0.0f, -1.0f, -1.f);
	pLights->m_pLights[0].m_bShadow = true;

	pLights->m_pLights[1].m_bEnable = true;
	pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	pLights->m_pLights[1].m_fRange = 500.0f;
	pLights->m_pLights[1].m_xmf4Ambient = Vector4(0.3f, 0.3f, 0.3f, 1.0f);
	pLights->m_pLights[1].m_xmf4Diffuse = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	pLights->m_pLights[1].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[1].m_xmf3Position = Vector3(-50.0f, 20.0f, -5.0f);
	pLights->m_pLights[1].m_xmf3Direction = Vector3(0.0f, 0.0f, 1.0f);
	pLights->m_pLights[1].m_xmf3Attenuation = Vector3(1.0f, 0.01f, 0.0001f);
	pLights->m_pLights[1].m_fFalloff = 8.0f;
	pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	pLights->m_pLights[2].m_bEnable = true;
	pLights->m_pLights[2].m_nType = SPOT_LIGHT;
	pLights->m_pLights[2].m_fRange = 500.0f;
	pLights->m_pLights[2].m_xmf4Ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	pLights->m_pLights[2].m_xmf4Diffuse = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	pLights->m_pLights[2].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[2].m_xmf3Position = Vector3(-50.0f, 20.0f, -5.0f);
	pLights->m_pLights[2].m_xmf3Direction = Vector3(0.0f, 0.0f, 1.0f);
	pLights->m_pLights[2].m_xmf3Attenuation = Vector3(1.0f, 0.01f, 0.0001f);
	pLights->m_pLights[2].m_fFalloff = 10.f;
	pLights->m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	pLights->m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	pLights->m_pLights[2].m_bShadow = true;

	pLights->m_pLights[3]. m_bEnable = true;
	pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	pLights->m_pLights[3].m_fRange = 500.0f;
	pLights->m_pLights[3].m_xmf4Ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	pLights->m_pLights[3].m_xmf4Diffuse = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	pLights->m_pLights[3].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[3].m_xmf3Position = Vector3(-50.0f, 20.0f, -5.0f);
	pLights->m_pLights[3].m_xmf3Direction = Vector3(0.0f, 0.0f, 1.0f);
	pLights->m_pLights[3].m_xmf3Attenuation = Vector3(1.0f, 0.01f, 0.0001f);
	pLights->m_pLights[3].m_fFalloff = 10.0f;
	pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	pLights->m_pLights[3].m_bShadow = true;

	pLights->m_pLights[4].m_bEnable = true;
	pLights->m_pLights[4].m_nType = SPOT_LIGHT;
	pLights->m_pLights[4].m_fRange = 500.0f;
	pLights->m_pLights[4].m_xmf4Ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	pLights->m_pLights[4].m_xmf4Diffuse = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	pLights->m_pLights[4].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[4].m_xmf3Position = Vector3(-50.0f, 20.0f, -5.0f);
	pLights->m_pLights[4].m_xmf3Direction = Vector3(0.0f, 0.0f, 1.0f);
	pLights->m_pLights[4].m_xmf3Attenuation = Vector3(1.0f, 0.01f, 0.0001f);
	pLights->m_pLights[4].m_fFalloff = 8.0f;
	pLights->m_pLights[4].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	pLights->m_pLights[4].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	pLights->m_pLights[4].m_bShadow = true;

	pLights->m_pLights[5].m_bEnable = false;
	pLights->m_pLights[5].m_nType = POINT_LIGHT;
	pLights->m_pLights[5].m_fRange = 300.0f;
	pLights->m_pLights[5].m_xmf4Ambient = Vector4(0.8f, 0.0f, 0.0f, 1.0f);
	pLights->m_pLights[5].m_xmf4Diffuse = Vector4(0.7f, 0.0f, 0.0f, 1.0f);
	pLights->m_pLights[5].m_xmf4Specular = Vector4(0.3f, 0.3f, 0.3f, 0.0f);
	pLights->m_pLights[5].m_xmf3Position = Vector3(2100, 300.0f, 2970.0f);
	pLights->m_pLights[5].m_xmf3Attenuation = Vector3(0.1f, 0.001f, 0.00005f);

	pLights->m_pLights[6].m_bEnable = false;
	pLights->m_pLights[6].m_nType = POINT_LIGHT;
	pLights->m_pLights[6].m_fRange = 300.0f;
	pLights->m_pLights[6].m_xmf4Ambient = Vector4(0.8f, 0.0f, 0.0f, 1.0f);
	pLights->m_pLights[6].m_xmf4Diffuse = Vector4(0.7f, 0.0f, 0.0f, 1.0f);
	pLights->m_pLights[6].m_xmf4Specular = Vector4(0.9f, 0.9f, 0.9f, 0.0f);
	pLights->m_pLights[6].m_xmf3Position = Vector3(2100, 300.0f, 3470.0f);
	pLights->m_pLights[6].m_xmf3Attenuation = Vector3(0.1f, 0.001f, 0.00005f);

	pLights->m_pLights[7].m_bEnable = false;
	pLights->m_pLights[7].m_nType = POINT_LIGHT;
	pLights->m_pLights[7].m_fRange = bossStagePointRange;
	pLights->m_pLights[7].m_xmf4Ambient = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
	pLights->m_pLights[7].m_xmf4Diffuse = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	pLights->m_pLights[7].m_xmf4Specular = Vector4(0.9f, 0.9f, 0.9f, 0.0f);
	pLights->m_pLights[7].m_xmf3Position = Vector3(310.0f, 1200.0f, 1230.0f);
	pLights->m_pLights[7].m_xmf3Attenuation = Vector3(0.01f, 0.001f, 0.0001f);

	pLights->m_pLights[8].m_bEnable = false;
	pLights->m_pLights[8].m_nType = POINT_LIGHT;
	pLights->m_pLights[8].m_fRange = bossStagePointRange;
	pLights->m_pLights[8].m_xmf4Ambient = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
	pLights->m_pLights[8].m_xmf4Diffuse = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	pLights->m_pLights[8].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[8].m_xmf3Position = Vector3(310.0f, 1200.0f, 770.0f);
	pLights->m_pLights[8].m_xmf3Attenuation = Vector3(0.01f, 0.001f, 0.0001f);

	pLights->m_pLights[9].m_bEnable = false;
	pLights->m_pLights[9].m_nType = POINT_LIGHT;
	pLights->m_pLights[9].m_fRange = bossStagePointRange;
	pLights->m_pLights[9].m_xmf4Ambient = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
	pLights->m_pLights[9].m_xmf4Diffuse = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	pLights->m_pLights[9].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[9].m_xmf3Position = Vector3(310.0f, 1200.0f, 310.0f);
	pLights->m_pLights[9].m_xmf3Attenuation = Vector3(0.01f, 0.001f, 0.0001f);

	pLights->m_pLights[10].m_bEnable = false;
	pLights->m_pLights[10].m_nType = POINT_LIGHT;
	pLights->m_pLights[10].m_fRange = bossStagePointRange;
	pLights->m_pLights[10].m_xmf4Ambient = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
	pLights->m_pLights[10].m_xmf4Diffuse = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
	pLights->m_pLights[10].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[10].m_xmf3Position = Vector3(770.0f, 1200.0f, 310.0f);
	pLights->m_pLights[10].m_xmf3Attenuation = Vector3(0.01f, 0.001f, 0.0001f);

	pLights->m_pLights[11].m_bEnable = false;
	pLights->m_pLights[11].m_nType = POINT_LIGHT;
	pLights->m_pLights[11].m_fRange = bossStagePointRange;
	pLights->m_pLights[11].m_xmf4Ambient = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
	pLights->m_pLights[11].m_xmf4Diffuse = Vector4(1.0f, 0.0f, 1.0f, 1.0f);
	pLights->m_pLights[11].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[11].m_xmf3Position = Vector3(1230.0f, 1200.0f, 310.0f);
	pLights->m_pLights[11].m_xmf3Attenuation = Vector3(0.01f, 0.001f, 0.0001f);

	pLights->m_pLights[12].m_bEnable = false;
	pLights->m_pLights[12].m_nType = POINT_LIGHT;
	pLights->m_pLights[12].m_fRange = bossStagePointRange;
	pLights->m_pLights[12].m_xmf4Ambient = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
	pLights->m_pLights[12].m_xmf4Diffuse = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
	pLights->m_pLights[12].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[12].m_xmf3Position = Vector3(1230.0f, 1200.0f, 770.0f);
	pLights->m_pLights[12].m_xmf3Attenuation = Vector3(0.01f, 0.001f, 0.0001f);

	pLights->m_pLights[13].m_bEnable = false;
	pLights->m_pLights[13].m_nType = POINT_LIGHT;
	pLights->m_pLights[13].m_fRange = bossStagePointRange;
	pLights->m_pLights[13].m_xmf4Ambient = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
	pLights->m_pLights[13].m_xmf4Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pLights->m_pLights[13].m_xmf4Specular = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	pLights->m_pLights[13].m_xmf3Position = Vector3(1230.0f, 1200.0f, 1230.0f);
	pLights->m_pLights[13].m_xmf3Attenuation = Vector3(0.01f, 0.001f, 0.0001f);

	gsLights = new CLight(pLights);

	gsMaterials = new MATERIALS;
	::ZeroMemory(gsMaterials, sizeof(MATERIALS));

	gsMaterials->m_pReflections[0] = { Vector4(1.0f, 1.0f, 1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 20.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f) };
	gsMaterials->m_pReflections[1] = { Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 10.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f) };
	gsMaterials->m_pReflections[2] = { Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 10.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f)  };
	gsMaterials->m_pReflections[3] = { Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 10.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f)  };
	gsMaterials->m_pReflections[4] = { Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(0.5f, 0.5f, 0.5f, 10.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f)  };

}

void SceneGame::Release()
{
	if (gsRootSignature) gsRootSignature->Release();

	if (gsPlayer)
	{
		gsPlayer->~CPlayer();
	}
	for (auto& i : gsMonster)
	{
		i->~CMovableHierarchyObject();
	}
	for (auto& i : gsOtherPlayer)
	{
		i->~CTestPlayerModel();
	}
	for (auto& a : gsMapObject)
	{
		a->ReleaseShaderVariables();
		a->ReleaseObject();
		a->Release();		
	}
	if (gsLights)
	{
		gsLights->ReleaseShaderVariables();
		delete gsLights;
	}

	if (gsTerrain) delete gsTerrain;
	if (gsSkyBox) delete gsSkyBox;
	ReleaseShaderVariables();

	if (gsMaterials) delete gsMaterials;
}

void SceneGame::ReleaseUploadBuffers()
{
	if (gsShotgunPlayer) gsShotgunPlayer->ReleaseUploadBuffers();
	if (gsSniperPlayer) gsSniperPlayer->ReleaseUploadBuffers();
	if (gsRiflePlayer) gsRiflePlayer->ReleaseUploadBuffers();
	if (gsTerrain) gsTerrain->ReleaseUploadBuffers();
	if (gsSkyBox) gsSkyBox->ReleaseUploadBuffers();
	for (auto& a : gsMapObject)
	{
		a->ReleaseUploadBuffers();
	}
	for (auto& a : gsOtherPlayer)
	{
		a->ReleaseUploadBuffers();
	}
	for (auto& a :gsMonster)
	{
		a->ReleaseUploadBuffers();
	}
}

void SceneGame::CheckinteractionObject()
{
	InteractionObject* NearestObject = NULL;
	NearestObject = PickObjectByCameraLookVectorForChest();
	if (NearestObject)
	{
		for (auto& door : gsInteractObjects)
		{
			if (door->GetName() == NearestObject->GetName())
				door->Interact();
		}
	}
}

void SceneGame::CheckinteractionItem()
{
	Item* NearestObject = NULL;
	NearestObject = PickObjectItem();
	if (NearestObject)
	{
		NearestObject->SetIsAlive(false);
		gsItemPacket.doSend = true;
		snd->PlayShoot(SOUND_TYPE::ITEM_PICKUP_SOUND, 1.0);
	}
}

void SceneGame::ShootingBullet()
{
	if (gsPlayer->GetBullet() != 0 && !gsPlayer->CheckReload())
	{
		currentTime = std::chrono::system_clock::now();
		std::chrono::duration<double> sec = currentTime - deltaTime;
		if (sec.count() > 1)reboundCount = 0;
		if (sec.count() > gsPlayer->GetAttackSpeed())
		{
			if (!(gsEffects[0]->GetIsAlive()))gsEffects[0]->SetIsAlive(true);

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dis(0, 100);

			rotateX = (-(80.f / (100 - reboundCount))) / 5.f;
			rotateY = (static_cast<float>(dis(gen)) / 50.f - 1.f) / 5.f;

			deltaTime = currentTime;
			reboundCount = min(80, reboundCount + 7);
			packet_type = CS_SHOOT_PACKET;
			switch (gsWeaponType)
			{
			case WEAPON_RIFLE:
				snd->PlayShoot(RIFLE_SOUND, 0.15);
				break;
			case WEAPON_SNIPER:
				snd->PlayShoot(SNIPER_SOUND, 0.15);
				break;
			case WEAPON_SHOTGUN:
				snd->PlayShoot(SHOTGUN_SOUND, 0.15);
				break;
			default:
				break;
			}
		}

	}
}

void SceneGame::PrintBoundingBox()
{
}

InteractionObject* SceneGame::PickObjectByCameraLookVectorForChest()
{
	bool			isIntersected = false;
	float			fHitDistance = 75.0f, fNearestHitDistance = 150.0f;
	InteractionObject* pNearestObject = NULL;
	for (auto& door : gsInteractObjects)
	{
		isIntersected = door->ColideObjectByRayIntersection(gsCamera->GetPosition(), gsCamera->GetLookVector(), &fHitDistance);
		if (isIntersected && (fHitDistance < fNearestHitDistance))
		{
			fNearestHitDistance = fHitDistance;
			pNearestObject = door;
		}
	}
	return pNearestObject;
}

Item* SceneGame::PickObjectItem()
{
	bool			isIntersected = false;
	float			fHitDistance = 75.0f, fNearestHitDistance = 150.0f;
	Item* pNearestObject = NULL;

	for (UINT i = 0; i < CHESTOBJECT_NUM; ++i)
	{
		for (UINT j = 0; j < gsInteractObjects[i]->GetItem().size(); ++j)
		{
			if (!gsInteractObjects[i]->GetItem().at(j)->GetIsAlive()) continue;
			isIntersected = gsInteractObjects[i]->GetItem().at(j)->ColideObjectByRayIntersection(gsCamera->GetPosition(), gsCamera->GetLookVector(), &fHitDistance);
			if (isIntersected && (fHitDistance < fNearestHitDistance))
			{
				fNearestHitDistance = fHitDistance;
				pNearestObject = gsInteractObjects[i]->GetItem().at(j);
				gsItemPacket.chestId = i;
				gsItemPacket.itemId = j;
			}
		}
	}
	return pNearestObject;
}

void SceneGame::ReloadAmmo()
{
	gsPlayer->rootObject->AnimationChange(9);
}

void SceneGame::InitWeapon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, WEAPON_TYPE type)
{
	switch (type)
	{
	case WEAPON_RIFLE:
		gsPlayer = gsRiflePlayer;
		gsCamera = gsPlayer->GetCamera();
		delete(gsSniperPlayer);
		delete(gsShotgunPlayer);
		break;
	case WEAPON_SNIPER:
		gsPlayer = gsSniperPlayer;
		gsCamera = gsPlayer->GetCamera();
		delete(gsRiflePlayer);
		delete(gsShotgunPlayer);
		break;
	case WEAPON_SHOTGUN:
		gsPlayer = gsShotgunPlayer;
		gsCamera = gsPlayer->GetCamera();
		delete(gsSniperPlayer);
		delete(gsRiflePlayer);
		break;
	default:
		break;
	}
	snd->Play(TITLE_BGM);
	snd->SetVolume(TITLE_BGM, 0.07f);
	gsWeaponType = type;
}

void SceneGame::CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256�� ���
	gsCbMaterials = ::CreateBufferResource(device, cmdList, NULL, ncbMaterialBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	gsCbMaterials->Map(0, NULL, (void**)&gsCbMappedMaterials);

	UINT ncbEnvironmentBytes = ((sizeof(ENVIRONMENT) + 255) & ~255); //256�� ���
	gsCbEnvironment = ::CreateBufferResource(device, cmdList, NULL, ncbEnvironmentBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	gsCbEnvironment->Map(0, NULL, (void**)&gsCbMappedEnvironment);

	UINT ncbLavaInfoBytes = ((sizeof(LAVAINFO) + 255) & ~255); //256�� ���
	gsCbLavaInfo = ::CreateBufferResource(device, cmdList, NULL, ncbLavaInfoBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	gsCbLavaInfo->Map(0, NULL, (void**)&gsCbMappedLavaInfo);

	UINT ncbElementBytes = ((sizeof(VS_CB_SHADOWCAMERA_INFO) + 255) & ~255); //256의 배수
	gsCbShadowMaps = ::CreateBufferResource(device, cmdList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	gsCbShadowMaps->Map(0, NULL, (void**)&gsCbMappedShadowMaps);
}

void SceneGame::ReleaseShaderVariables()
{
	if (gsCbMaterials)
	{
		gsCbMaterials->Unmap(0, NULL);
		gsCbMaterials->Release();
	}
	if (gsCbEnvironment)
	{
		gsCbEnvironment->Unmap(0, NULL);
		gsCbEnvironment->Release();
	}
	if (gsCbLavaInfo)
	{
		gsCbLavaInfo->Unmap(0, NULL);
		gsCbLavaInfo->Release();
	}
}

void SceneGame::CreateShadowMap(ID3D12Device* pd3dDevice)
{
	gsShadowMaps.reserve(MAX_SHADOWMAP);
	gsShadowMaps.push_back(make_unique<ShadowMap>(pd3dDevice, 7680, 4320));
	for (UINT i = 0; i < MAX_SHADOWMAP - 1; i++)
		gsShadowMaps.push_back(make_unique<ShadowMap>(pd3dDevice, 1024, 1024));
}

void SceneGame::CreateShadowDescriptorHeap(ID3D12Device* device)
{
	HRESULT hResult;
	D3D12_DESCRIPTOR_HEAP_DESC ShadowSrvHeapDesc;
	ShadowSrvHeapDesc.NumDescriptors = MAX_SHADOWMAP + 5; //�÷��̾� �� (shadowmap ��)
	ShadowSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ShadowSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ShadowSrvHeapDesc.NodeMask = 0;
	hResult = device->CreateDescriptorHeap(&ShadowSrvHeapDesc, IID_PPV_ARGS(&gsShadowSrvDescHeap));
	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuStart;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuStart;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuStart;
	for (int i = 0; i < MAX_SHADOWMAP; i++)
	{
		srvCpuStart = gsShadowSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
		srvGpuStart = gsShadowSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
		dsvCpuStart = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		dsvCpuStart.ptr += (gnDsvDescriptorIncrementSize * (i + 1));
		srvCpuStart.ptr += (gnCbvSrvDescriptorIncrementSize * i);
		srvGpuStart.ptr += (gnCbvSrvDescriptorIncrementSize * i);
		gsShadowMaps[i]->BuildDescriptors(srvCpuStart, srvGpuStart, dsvCpuStart);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuStart;
	rtvCpuStart = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCpuStart.ptr += gnRtvDescriptorIncrementSize * 2;
	srvCpuStart.ptr += (gnCbvSrvDescriptorIncrementSize);
	srvGpuStart.ptr += (gnCbvSrvDescriptorIncrementSize);
	gsSsao->BuildDescriptors(dsvBuffer, srvCpuStart, srvGpuStart, rtvCpuStart, gnCbvSrvDescriptorIncrementSize, gnRtvDescriptorIncrementSize);
}

void SceneGame::SetShadowCamera()
{
	shadowMapViews.clear();
	shadowMapProjs.clear();
	LIGHT light = gsLights->m_pLights->m_pLights[0]; //이거가지고 스팟만들기
	Matrix4x4 matLightProj, matLightView;
	XMVECTOR lightDir = XMLoadFloat3(&light.m_xmf3Direction);
	XMVECTOR lightPos = 1.5f *6000 * -lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&Vector3(2500, 400, 2500));
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));
	//직교투영 디렉셔널라이트
	float l = sphereCenterLS.x - 3000;
	float b = sphereCenterLS.y - 3000;
	float n = sphereCenterLS.z - 3000;
	float r = sphereCenterLS.x + 3000;
	float t = sphereCenterLS.y + 3000;
	float f = sphereCenterLS.z + 3000;

	float mLightNearZ = n;
	float mLightFarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	XMStoreFloat4x4(&(matLightView), lightView);
	XMStoreFloat4x4(&(matLightProj), lightProj);
	gsShadowMaps[0]->SetShadowMatrix(matLightView, matLightProj);
	shadowMapViews.push_back(matLightView.transpose());
	shadowMapProjs.push_back(matLightProj.transpose());

	for (UINT i = 0; i < MAX_SHADOWMAP - 1; i++) {
		light = gsLights->m_pLights->m_pLights[i + 2];

		Vector3 up = Vector3(0, 1, 0);
		Vector3 look = light.m_xmf3Direction.normalized();
		Vector3 right = Vector3::CrossNormal(look, up);
		up = Vector3::CrossNormal(look, right);
		float znzf = 500.f / (500.f - 0.001f);
		matLightView = {
			right.x, up.x, look.x, 0,
			right.y, up.y, look.y, 0,
			right.z, up.z, look.z, 0,
			-Vector3::Dot(light.m_xmf3Position, right),	-Vector3::Dot(light.m_xmf3Position, up),-Vector3::Dot(light.m_xmf3Position, look),1
		};
		matLightProj = Matrix4x4::Perspective(XMConvertToRadians(120), 1, 2, 300);
		gsShadowMaps[i + 1]->SetShadowMatrix(matLightView, matLightProj);
		shadowMapViews.push_back(matLightView.transpose());
		shadowMapProjs.push_back(matLightProj.transpose());
	}
}

void SceneGame::UpdateShaderVariables(ID3D12GraphicsCommandList* cmdList)
{
	::memcpy(gsCbMappedMaterials, gsMaterials, sizeof(MATERIALS));

	::memcpy(gsCbMappedLavaInfo, gsLavaInfo, sizeof(LAVAINFO));
	gsCbMappedEnvironment->drawFog = gsDrawfog;
	gsCbMappedEnvironment->fogPos = gsFogPos;

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = gsCbMaterials->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(MATERIAL_CBV, d3dcbMaterialsGpuVirtualAddress); //Materials
	D3D12_GPU_VIRTUAL_ADDRESS cbEnvironmentGpuVirtualAddress = gsCbEnvironment->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(ENVIROMENT_CBV, cbEnvironmentGpuVirtualAddress);
	D3D12_GPU_VIRTUAL_ADDRESS cbLavaInfoGpuVirtualAddress = gsCbLavaInfo->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(LAVAWAVE_CBV, cbLavaInfoGpuVirtualAddress);

	::memcpy(gsCbMappedShadowMaps->shadowViews, shadowMapViews.data(), sizeof(Matrix4x4) * shadowMapViews.size());
	::memcpy(gsCbMappedShadowMaps->shadowProjs, shadowMapProjs.data(), sizeof(Matrix4x4) * shadowMapProjs.size());

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = gsCbShadowMaps->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(SHADOWMAPS_CBV, d3dGpuVirtualAddress);
}

void SceneGame::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&oldCursorPos);
		break;
	case WM_RBUTTONDOWN:
		if (gsWeaponType == WEAPON_SNIPER)
		{
			gsPlayer->CameraZoomInOut();
			gsEffects.at(11)->SetIsAlive(!gsEffects.at(11)->GetIsAlive());
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON)
		{
			ShootingBullet();
		}
		break;
	default:
		break;
	}
}

void SceneGame::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	WPARAM key_buffer = wParam;
	switch (nMessageID)
	{
	case WM_KEYDOWN:
	{
		switch (key_buffer)
		{
		case 'w':
		case 'W':
			if (gsKeyInput.Key_W == false)
				gsKeyInput.Key_W = true;
			break;

		case 's':
		case 'S':
			if (gsKeyInput.Key_S == false)
				gsKeyInput.Key_S = true;
			break;

		case 'a':
		case 'A':
			if (gsKeyInput.Key_A == false)
				gsKeyInput.Key_A = true;
			break;

		case 'd':
		case 'D':
			if (gsKeyInput.Key_D == false)
				gsKeyInput.Key_D = true;
			break;
		case 'E':
		case 'e':
			if (gsKeyInput.Key_E == false)
				gsKeyInput.Key_E = true;
			CheckinteractionItem();
			break;
		case 'r':
		case 'R':
			if (gsKeyInput.Key_R == false)
				gsKeyInput.Key_R = true;
			break;
		case 'q':
		case 'Q':
			if (gsKeyInput.Key_Q == false)
				gsKeyInput.Key_Q = true;
			gsPlayer->SetActiveItemAlive(false);
			break;
		case 'n':
		case 'N':
			if (gsKeyInput.Key_N == false)
				gsKeyInput.Key_N = true;
			break;
		case 'm':
		case 'M':
			if (gsKeyInput.Key_M == false)
				gsKeyInput.Key_M = true;
			break;
		case 'b':
		case 'B':
			if (gsKeyInput.Key_B == false)
				gsKeyInput.Key_B = true;
			break;
		case 'u':
		case 'U':
			gsDrawfog = !gsDrawfog;
			gsFogPos = 300;
			break;
		case 't':
		case 'T':
			break;
		case VK_TAB:
			gsPlayer->DrawInventory(true);
			break;
		default:
			break;
		}
		break;
	}
	case WM_KEYUP:
	{
		switch (key_buffer)
		{
		case 'w':
		case 'W':
			if (gsKeyInput.Key_W == true)
				gsKeyInput.Key_W = false;
			break;

		case 's':
		case 'S':
			if (gsKeyInput.Key_S == true)
				gsKeyInput.Key_S = false;
			break;

		case 'a':
		case 'A':
			if (gsKeyInput.Key_A == true)
				gsKeyInput.Key_A = false;
			break;

		case 'd':
		case 'D':
			if (gsKeyInput.Key_D == true)
				gsKeyInput.Key_D = false;
			break;
		case 'E':
		case 'e':
			if (gsKeyInput.Key_E == true)
				gsKeyInput.Key_E = false;
			break;
		case 'r':
		case 'R':
			if (gsKeyInput.Key_R == true)
				gsKeyInput.Key_R = false;

		case 'q':
		case 'Q':
			if (gsKeyInput.Key_Q == true)
				gsKeyInput.Key_Q = false;
		case 'n':
		case 'N':
			if (gsKeyInput.Key_N == true)
				gsKeyInput.Key_N = false;
			break;
		case 'm':
		case 'M':
			if (gsKeyInput.Key_M == true)
				gsKeyInput.Key_M = false;
			break;
		case 'b':
		case 'B':
			if (gsKeyInput.Key_B == true)
				gsKeyInput.Key_B = false;
			break;
		case VK_TAB:
			gsPlayer->DrawInventory(false);
			break;
		default:
			break;
		}
	}
	}
}

void SceneGame::ProcessInput(HWND hWnd, float timeElapsed, ServerMgr* servermgr)
{
	static UCHAR pKeysBuffer[256];
	float cxDelta = 0.0f, cyDelta = 0.0f;
	bool bProcessedByScene = false;


	if (!bProcessedByScene)
	{
		DWORD dwDirection = 0;

		POINT ptCursorPos;
		if (GetCapture() == hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);

			cxDelta = (float)(ptCursorPos.x - oldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - oldCursorPos.y) / 3.0f;
			SetCursorPos(oldCursorPos.x, oldCursorPos.y);

		}


		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				gsPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
		}
	}
	servermgr->CSpacket_ingame.type = packet_type;
	gsPlayer->Update(timeElapsed);
	
}


void SceneGame::Update(float timeElapsed, ServerMgr* servermgr)
{
	timer = timeElapsed;
	
	snd->Update();

	servermgr->CSpacket_ingame.look = gsPlayer->GetLook();
	servermgr->CSpacket_ingame.cameraLook = gsCamera->GetLookVector();
	servermgr->CSpacket_ingame.input = gsKeyInput;
	servermgr->CSpacket_ingame.item = gsItemPacket;
	servermgr->CSpacket_ingame.id = servermgr->myID;
	gsItemPacket.doSend = false;
	packet_type = CS_INGAME_PACKET;
	//서버 패킷기반 업데이트
	if (servermgr->id_init == false) {
		playerID = servermgr->myID;
		servermgr->CSpacket_ingame.id = servermgr->myID;

		for (UINT i = 0, x = 0; i < MAX_PLAYER; i++)
		{
			if (i == playerID) continue;

			if (gsOtherPlayer[x]->Player_ID == 99) gsOtherPlayer[x]->Player_ID = i;
			x++;
		}

		servermgr->id_init = true;
	}

	//서버 패킷기반 업데이트
	switch (servermgr->SCpacket_ingame.type)
	{
	case SC_SET_ID_PACKET:
	{
		break;
	}
	case SC_GAME_TO_ENDING_PACKET:
		sceneChangeFlag = true;
		break;
	default:
	{
		gsPlayer->SetPosition(servermgr->SCpacket_ingame.player[servermgr->myID].pos);
		gsPlayer->SetHp(servermgr->SCpacket_ingame.player[servermgr->myID].ps.hp, servermgr->SCpacket_ingame.player[servermgr->myID].ps.maxHp);
		gsPlayer->SetBullet(servermgr->SCpacket_ingame.player[servermgr->myID].ammo);
		gsPlayer->SetAttackSpeed(servermgr->SCpacket_ingame.player[servermgr->myID].ps.attackSpeed);
		gsZoneNum = servermgr->SCpacket_ingame.player[servermgr->myID].zoneNum;

		gsPlayer->Update(timer);

		if (servermgr->SCpacket_ingame.player[servermgr->myID].currentItem != ITEM_EMPTY)
		{
			gsPlayer->SetItem(servermgr->SCpacket_ingame.player[servermgr->myID].currentItem);
		}

		if (servermgr->SCpacket_ingame.player[servermgr->myID].reloadEnable)
		{
			ReloadAmmo();
			
			switch (gsWeaponType)
			{
			case WEAPON_RIFLE:
				snd->PlayReload(RIFLE_RELOAD_SOUND, 0.3);
				break;
			case WEAPON_SNIPER:
				snd->PlayReload(RIFLE_RELOAD_SOUND, 0.5);
				break;
			case WEAPON_SHOTGUN:
				snd->PlayReload(SHOTGUN_RELOAD_SOUND, 0.3);
				break;
			default:
				break;
			}
		}


		for (auto& i : gsOtherPlayer)
		{
			Vector3 pos = servermgr->SCpacket_ingame.player[i->Player_ID].pos;
			Vector3 look = servermgr->SCpacket_ingame.player[i->Player_ID].look;
			Vector3 cameraLook = servermgr->SCpacket_ingame.player[i->Player_ID].cameraLook;

			i->SetServerData(pos, look, cameraLook);
			i->GetFbxObject()->AnimationChange(servermgr->SCpacket_ingame.player[i->Player_ID].state);
			i->Update(timeElapsed, gsCamera);
		}

		for (UINT i =0; i< gsMonster.size(); ++i)
		{
			auto it = find(gsZone.at(gsZoneNum).monsterID.begin(), gsZone.at(gsZoneNum).monsterID.end(), i);
			if (it == gsZone.at(gsZoneNum).monsterID.end()) continue;
			Vector3 pos = servermgr->SCpacket_ingame.npc[i].pos;
			Vector3 look = servermgr->SCpacket_ingame.npc[i].look;
			UINT hp = servermgr->SCpacket_ingame.npc[i].hp;
		

			gsMonster[i]->SetServerData(pos, look, look);
			gsMonster[i]->GetFbxObject()->AnimationChange(servermgr->SCpacket_ingame.npc[i].state);
			gsMonster[i]->SetHp(hp);
			if(servermgr->SCpacket_ingame.npc[i].attackEnable)
				gsMonster[i]->Attack();
			gsMonster[i]->Update(timeElapsed, gsCamera);
		}

		for (UINT i = 0; i < gsInteractObjects.size(); ++i)
		{
			Vector3 pos = servermgr->SCpacket_ingame.interaction[i].pos;
			Vector3 look = servermgr->SCpacket_ingame.interaction[i].look;
		
			gsInteractObjects[i]->SetServerData(pos, look, look);
			for (int j = 0; j < 4; ++j)
			{
				gsInteractObjects[i]->SetItem(j, servermgr->SCpacket_ingame.interaction[i].item[j].itemType);
				gsInteractObjects[i]->SetItemAlive(j, servermgr->SCpacket_ingame.interaction[i].item[j].isAlive);
			}
			if (servermgr->SCpacket_ingame.interaction[i].interactEnable)
				gsInteractObjects[i]->Interact();
			gsInteractObjects[i]->Update(timeElapsed, gsCamera);
		}

		for (int i = 0; i < gsStones.size(); ++i)
		{
			gsStones.at(i)->SetIsAlive(servermgr->SCpacket_ingame.stone[i].activeEnable);
			gsStones.at(i)->Move(servermgr->SCpacket_ingame.stone[i].pos);
		}

		for (int i = 0; i < gsEffectRangeAttack.size(); ++i)
		{
			gsEffectRangeAttack.at(i)->SetIsAlive(servermgr->SCpacket_ingame.attack[i].activeEnable);
			gsEffectRangeAttack.at(i)->SetPosition(servermgr->SCpacket_ingame.attack[i].pos);
		}
		for (int j = 0; j < 5; ++j)
		{
			if (servermgr->SCpacket_ingame.player[servermgr->myID].bullet[j].in_use)
			{
				Vector3 colPos = servermgr->SCpacket_ingame.player[servermgr->myID].bullet[j].pos;
				Matrix4x4 colWorld = Matrix4x4::identity;
				colWorld._41 = colPos.x;
				colWorld._42 = colPos.y;
				colWorld._43 = colPos.z;
				obj_type colType = servermgr->SCpacket_ingame.player[servermgr->myID].bullet[j].type;
				if (colType == type_static)
				{
					for (int i = 1; i < 6; ++i)
					{
						if (!(gsEffects[i]->GetIsAlive()))
						{
							gsEffects[i]->SetWorldMatrix(colWorld);
							gsEffects[i]->SetIsAlive(true);
							break;
						}
					}
					for (auto& p : gsEffectsDecal)
						{
						if (!p->GetIsAlive())
						{
							p->SetCameraPos(gsCamera->GetPosition());
							p->SetWorldMatrix(colWorld);
							break;
						}
					}
				}
				else
				{
					for (int i = 6; i < 11; ++i)
					{
						if (!(gsEffects[i]->GetIsAlive()))
						{
							gsEffects[i]->SetWorldMatrix(colWorld);
							gsEffects[i]->SetIsAlive(true);
							break;
						}
					}
				}
			}
		}
		servermgr->CSpacket_ingame.type = packet_type;
		servermgr->CSpacket_ingame.id = servermgr->myID;

		break;
	}
	}
	
	for (auto a : gsEffects) a->Update(timeElapsed, gsCamera);
	for (auto a : gsInteractObjects) a->Update(timeElapsed, gsCamera);
	for (auto a : gsEffectsDecal) a->Update(timeElapsed, gsCamera);
	for (auto a : gsEffectRangeAttack) a->Update(timeElapsed, gsCamera);
	for (auto a : gsStones) a->Update(timeElapsed, gsCamera);

	//조명관련 업데이트
	if (gsLights)
	{
		if (bossStagePointRange >= 400.0f || bossStagePointRange <= 200.0f) bossStagePointAnimaition *= -1;
		bossStagePointRange += bossStagePointAnimaition;

		gsLights->setLightPosition(1, gsCamera->GetPosition()+ gsCamera->GetLookVector()*70); //플레이어 스포트라이트
		gsLights->setLightDirection(1, gsCamera->GetLookVector());
		for (int i = 0; i < MAX_PLAYER - 1; i++) // 다른플레이어 스포트라이트
		{
			gsLights->setLightPosition(i + 2, gsOtherPlayer[i]->GetPosition() + gsOtherPlayer[i]->GetLookVector() * 70);
			gsLights->setLightDirection(i + 2, gsOtherPlayer[i]->GetLookVector());
		}

		for (int i = 7; i < 14; ++i)
		{
			gsLights->setLightRange(i, bossStagePointRange);
		}
		gsLights->setGrayScale(gsPlayer->GetDrawGrayScale()); //그레이스케일 적용
	}

	//안개처리
	Vector3 pos = gsPlayer->GetPosition();
	if ((pos.x > 111 && pos.x < 1095) && (pos.z > 2030 && pos.z < 3660))
	{
		if (!gsDrawfog)
		{
			gsDrawfog = true;
			gsFogPos = 1000;
		}
		if(gsDrawfog)
			gsFogPos = max(100, gsFogPos - 10);
	}
	else
	{
		if (gsDrawfog)
			gsFogPos = min(1000, gsFogPos + 10);

		if (gsFogPos >= 1000)
			gsDrawfog = false;
	}

	//총알 반동처리
	currentTime = std::chrono::system_clock::now();
	std::chrono::duration<double> sec = currentTime - deltaTime;
	if (sec.count() < 0.07 && sec.count() > 0.01)
		gsPlayer->Rotate(rotateX, rotateY, 0);
	if (sec.count() < 0.3 && sec.count() > 0.1)
		gsPlayer->Rotate(0.05f, 0, 0);

	//용암 애니메이션
	gsLavaInfo->gTextureAnimation._32 += timeElapsed * 0.005f;
	gsLavaInfo->gTextureAnimation = gsLavaInfo->gTextureAnimation.transpose();
}


void SceneGame::ShadowRender(ID3D12GraphicsCommandList* cmdList)
{
	gsCamera->UpdateShaderVariables(cmdList);
	gsLights->UpdateShaderVariables(cmdList);
	UpdateShaderVariables(cmdList);

	gsTerrain->Render(cmdList, gsCamera);
	for (auto& a : gsMapObject) a->Render(cmdList, gsCamera);
	
	for (auto& a : gsOtherPlayer) a->Render(cmdList, gsCamera);

	for (UINT i = 0; i < gsMonster.size(); ++i)
	{
		auto it = find(gsZone.at(gsZoneNum).monsterID.begin(), gsZone.at(gsZoneNum).monsterID.end(), i);
		if (it == gsZone.at(gsZoneNum).monsterID.end()) continue;
		gsMonster[i]->Render(cmdList, gsCamera);
	}

	for (auto& a : gsStones) a->Render(cmdList, gsCamera);
	for (auto& a : gsInteractObjects) a->Render(cmdList, gsCamera);
}

void SceneGame::DrawNormalRender(ID3D12GraphicsCommandList* cmdList)
{
	gsCamera->SetViewportsAndScissorRects(cmdList);
	gsCamera->UpdateShaderVariables(cmdList);
	gsLights->UpdateShaderVariables(cmdList);
	UpdateShaderVariables(cmdList);

	gsPlayer->Render(cmdList, gsCamera);
	for (auto& a : gsMapObject) a->Render(cmdList, gsCamera);

	for (auto& a : gsOtherPlayer) a->Render(cmdList, gsCamera);

	for (UINT i = 0; i < gsMonster.size(); ++i)
	{
		auto it = find(gsZone.at(gsZoneNum).monsterID.begin(), gsZone.at(gsZoneNum).monsterID.end(), i);
		if (it == gsZone.at(gsZoneNum).monsterID.end()) continue;
		gsMonster[i]->Render(cmdList, gsCamera);
	}
	for (auto& a : gsStones) a->Render(cmdList, gsCamera);
	for (auto& a : gsInteractObjects) a->Render(cmdList, gsCamera);
}

void SceneGame::ComputeWaveRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	static float t_base = 0.0f;
	std::chrono::duration<float> sec = chrono::system_clock::now() - startTime;
	gsLavaMgr->SetDescriptorHeap(pd3dCommandList);
	if ((sec.count() - t_base) >= 0.1f)
	{
		t_base += 0.1f;

		int i = Mathf::RandF(4, gsLavaMgr->RowCount() - 5);
		int j = Mathf::RandF(4, gsLavaMgr->ColumnCount() - 5);

		float r = Mathf::RandF(3.0f, 10.0f);

		gsLavaMgr->Disturb(pd3dCommandList, i, j, r);
	}

	gsLavaMgr->Update(timer, pd3dCommandList);
}

void SceneGame::DrawNormalAndDepth(ID3D12GraphicsCommandList* cmdList)
{
	renderType = RENDER_TYPE::DRAW_NORMAL;
	auto normalMap = gsSsao->NormalMap();
	auto normalMapRtv = gsSsao->NormalMapRtv();
	
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
	float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };

	cmdList->ClearRenderTargetView(normalMapRtv, clearValue, 0, NULL);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cmdList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &normalMapRtv, true, &d3dDsvCPUDescriptorHandle);

	cmdList->SetGraphicsRootSignature(gsRootSignature);
	DrawNormalRender(cmdList);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
}

void SceneGame::RenderPostProcessing(ID3D12GraphicsCommandList* cmdList)
{
	ComputeWaveRender(cmdList);
	renderType = RENDER_TYPE::SHADOW_RENDER;
	SetShadowCamera();
	for (UINT i = 0; i < MAX_SHADOWMAP; i++)
	{
		cmdList->RSSetViewports(1, &gsShadowMaps[i]->Viewport());
		cmdList->RSSetScissorRects(1, &gsShadowMaps[i]->ScissorRect());

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gsShadowMaps[i]->Resource(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

		cmdList->ClearDepthStencilView(gsShadowMaps[i]->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(0, nullptr, false, &gsShadowMaps[i]->Dsv());

		cmdList->SetGraphicsRootSignature(gsRootSignature);
		gsShadowMaps[i]->UpdateShaderVariables(cmdList);
		ShadowRender(cmdList);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gsShadowMaps[i]->Resource(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
	}
	DrawNormalAndDepth(cmdList);

	cmdList->SetDescriptorHeaps(1, &gsShadowSrvDescHeap);

	gsSsao->UpdateShaderVariables(gsCamera);
	gsSsao->ComputeSsao(cmdList, 6);
}

void SceneGame::Render(ID3D12GraphicsCommandList *cmdList)
{
	cmdList->SetGraphicsRootSignature(gsRootSignature);
	gsCamera->SetViewportsAndScissorRects(cmdList);
	gsCamera->UpdateShaderVariables(cmdList);
	gsLights->UpdateShaderVariables(cmdList);
	UpdateShaderVariables(cmdList);

	cmdList->SetGraphicsRootDescriptorTable(SHADOWMAP_SRV, gsShadowMaps[0]->Srv());
	cmdList->SetGraphicsRootDescriptorTable(SSAOMAP_SRV, gsSsao->Srv());
	gsLavaMgr->SetDescriptorHeap(cmdList);
	cmdList->SetGraphicsRootDescriptorTable(LAVAWAVE_SRV, gsLavaMgr->DisplacementMap());

	gsSkyBox->Render(cmdList, gsCamera);
	gsTerrain->Render(cmdList, gsCamera);
	gsPlayer->Render(cmdList, gsCamera);
	for (auto& a : gsMapObject) a->Render(cmdList, gsCamera);

	for (auto& a : gsOtherPlayer) a->Render(cmdList, gsCamera);

	for (UINT i = 0; i < gsMonster.size(); ++i)
	{
		auto it = find(gsZone.at(gsZoneNum).monsterID.begin(), gsZone.at(gsZoneNum).monsterID.end(), i);
		if (it == gsZone.at(gsZoneNum).monsterID.end()) continue;
		gsMonster[i]->Render(cmdList, gsCamera);
	}

	for (auto& a : gsInteractObjects) a->Render(cmdList, gsCamera);

	for (auto& a : gsLavaObject) a->Render(cmdList, gsCamera);

	for (auto& a : gsEffects) a->Render(cmdList, gsCamera);

	for (auto& a : gsEffectsDecal)a->Render(cmdList, gsCamera);

	for (auto& a : gsEffectRangeAttack) a->Render(cmdList, gsCamera);

	for (auto& a : gsStones) a->Render(cmdList, gsCamera);
}