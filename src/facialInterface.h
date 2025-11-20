#include <string>
#include "raylib.h"
#include "raymath.h"
#include "vector"

class FacialInterface{
public:
	FacialInterface(std::string modlePath, Shader shader, int animCount);
	~FacialInterface();
	static ModelAnimation GenerateInBetween(ModelAnimation* anims, int index1, int index2, float t);
	static ModelAnimation BlendByVector(ModelAnimation* anims, const std::vector<float>& v);
	void Update();
	void Draw(Camera3D& cam);


private:
	Model faceModel;	
	ModelAnimation* anims;
};
