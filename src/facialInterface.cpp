#include "facialInterface.h"
#include <iostream>
#include <string>


static std::vector<float> NormalizeVector(const std::vector<float>& v);
static void LerpInPlace(std::vector<float>& current, const std::vector<float>& target, float t);

FacialInterface::FacialInterface(std::string modelPath, Shader shader, int animCount){
	faceModel = LoadModel(modelPath.c_str());
	faceModel.materials[0].shader = shader;
	anims = LoadModelAnimations("res/FaceModel.glb", &animCount);

	animationWeights = std::vector<float>();
	animationWeights.assign(anims->frameCount, 0.0f);
	animationWeights[0] = 1.0f;

	targetWeights = std::vector<float>();
	targetWeights.assign(anims->frameCount, 0.0f);
	targetWeights[0] = 1.0f;

	std::cout<< "frame count" << anims->frameCount <<std::endl;
}

FacialInterface::~FacialInterface(){}


void FacialInterface::Update(){
	UpdateModelAnimation(faceModel, BlendByVector(anims, animationWeights),0);
	LerpInPlace(animationWeights, targetWeights,GetFrameTime());
}

void FacialInterface::UpdateWeights(std::vector<float> weights){
    std::vector<float> temp;
    temp.reserve(weights.size());

    for (size_t i = 0; i < weights.size() && (i * 2) < targetWeights.size(); i++) {
        temp.push_back(weights[i]);
    }
    temp = NormalizeVector(temp);

    for (size_t i = 0; i < temp.size(); i++) {
        targetWeights[i * 2] = temp[i];
    }
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

static std::vector<float> NormalizeVector(const std::vector<float>& v) {
    float sum = 0.0f;
    for (float x : v) sum += x;

    if (sum == 0.0f) 
        return v;

    std::vector<float> out;
    out.reserve(v.size());

    for (float x : v) 
        out.push_back(x / sum);

    return out;
}


static void LerpInPlace(std::vector<float>& current, const std::vector<float>& target, float t) {
    size_t n = current.size();
    for (size_t i = 0; i < n; i++) {
        current[i] = current[i] + (target[i] - current[i]) * t;
    }
}
