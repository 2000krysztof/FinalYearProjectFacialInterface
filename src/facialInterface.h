#include <string>
#include "raylib.h"
#include "raymath.h"
#include "vector"


struct EmotionSegment {
    std::string emotion;
    float start;
    float end;
};

class FacialInterface{
public:
	FacialInterface(std::string modelPath, Shader shader, int animCount);
	~FacialInterface();
	static ModelAnimation GenerateInBetween(ModelAnimation* anims, int index1, int index2, float t);
	static ModelAnimation BlendByVector(ModelAnimation* anims, const std::vector<float>& v);
	void Update();
	void Draw(Camera3D& cam);
	void UpdateWeights(std::vector<float> weights);
	void SetTimeline(std::vector<EmotionSegment> timeline);

private:
	Model faceModel;	
	ModelAnimation* anims;
	void PlayTimeline();
	std::vector<float> EmotionToWeights(const std::string& emotion);
	std::vector<float> animationWeights;
	std::vector<float> targetWeights;
	std::vector<EmotionSegment> timeline;
	float playbackTime = 0.0f;
	size_t currentIndex = 0;
	bool playing = false;
};


