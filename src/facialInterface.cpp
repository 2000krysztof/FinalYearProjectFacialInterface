#include "facialInterface.h"
#include <iostream>
#include <string>


FacialInterface::FacialInterface(std::string modelPath, Shader shader, int animCount){
	faceModel = LoadModel(modelPath.c_str());
	faceModel.materials[0].shader = shader;
	anims = LoadModelAnimations("res/FaceModel.glb", &animCount);
}

FacialInterface::~FacialInterface(){}


void FacialInterface::Update(){
	//UpdateModelAnimation(faceModel, generateInBetween(anims,animIndex, animIndex+1,blend), 0);
}

void FacialInterface::Draw(Camera3D& cam){
	BeginDrawing();
	ClearBackground(BLACK);
	BeginMode3D(cam);
	DrawModel(faceModel, {0,0,0}, 0.1f, WHITE);

	EndMode3D();
	EndDrawing();

}


ModelAnimation FacialInterface::GenerateInBetween(ModelAnimation* anims, int index1, int index2, float t) {

    if (index1 >= anims->frameCount || index2 >= anims->frameCount) {
        return (ModelAnimation){ 0 };
    }

    Transform* keyFrameA = anims->framePoses[index1];
    Transform* keyFrameB = anims->framePoses[index2];


    ModelAnimation tweenAnim = { 0 };

    tweenAnim.name[0] = '\0';
    tweenAnim.boneCount = anims->boneCount;
    tweenAnim.frameCount = 1;

    tweenAnim.framePoses = (Transform**)RL_CALLOC(tweenAnim.frameCount, sizeof(Transform*));

    tweenAnim.framePoses[0] = (Transform*)RL_CALLOC(tweenAnim.boneCount, sizeof(Transform));

    tweenAnim.bones = anims->bones; 
    for (int i = 0; i < anims->boneCount; i++) {
        tweenAnim.framePoses[0][i].translation = Vector3Lerp(keyFrameA[i].translation, keyFrameB[i].translation, t);
        tweenAnim.framePoses[0][i].scale = Vector3Lerp(keyFrameA[i].scale, keyFrameB[i].scale, t);
        tweenAnim.framePoses[0][i].rotation = QuaternionLerp(keyFrameA[i].rotation, keyFrameB[i].rotation, t);
    }

    return tweenAnim;
}



ModelAnimation FacialInterface::BlendByVector(ModelAnimation* anims, const std::vector<float>& v){
    
    if (v.size() != anims->frameCount) {
        TraceLog(LOG_WARNING, "ANIMATION BLEND: Weight vector size (%d) does not match animation frame count (%d). Returning empty animation.", (int)v.size(), anims->frameCount);
        return (ModelAnimation){ 0 }; 
    }

    ModelAnimation tweenAnim = { 0 };
    tweenAnim.name[0] = '\0';
    tweenAnim.boneCount = anims->boneCount;
    tweenAnim.frameCount = 1;
    tweenAnim.framePoses = (Transform**)RL_CALLOC(tweenAnim.frameCount, sizeof(Transform*));
    tweenAnim.framePoses[0] = (Transform*)RL_CALLOC(tweenAnim.boneCount, sizeof(Transform));
    tweenAnim.bones = anims->bones;

    for (int i = 0; i < anims->boneCount; i++) {
        
        Vector3 blendedTranslation = { 0.0f, 0.0f, 0.0f };
        Vector3 blendedScale = { 0.0f, 0.0f, 0.0f };
        Quaternion blendedRotation = { 0.0f, 0.0f, 0.0f, 0.0f }; 

        for (int j = 0; j < v.size(); j++) {
            float weight = v[j];
            Transform* pose = anims->framePoses[j];
            blendedTranslation = Vector3Add(blendedTranslation, Vector3Scale(pose[i].translation, weight));
            blendedScale = Vector3Add(blendedScale, Vector3Scale(pose[i].scale, weight));
            blendedRotation = QuaternionAdd(blendedRotation, QuaternionScale(pose[i].rotation, weight));
        }
        
        blendedRotation = QuaternionNormalize(blendedRotation);
        
        tweenAnim.framePoses[0][i].translation = blendedTranslation;
        tweenAnim.framePoses[0][i].scale = blendedScale;
        tweenAnim.framePoses[0][i].rotation = blendedRotation;
    }
    
    return tweenAnim;
}
