#include "olcEngine.h";
#include <fstream>;
#include <strstream>;
#include <algorithm>;
 using namespace std;

struct vec3 {
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};
struct triangle {
    vec3  vert[3];
	float light_incidence;
};
struct mesh {

    vector<triangle> tris;

	bool LoadObjectFile(string filename) {
		//ifstream executes input and output operations 
		//On the file it is associated with
		ifstream f(filename);
		if (!f.is_open())
			return false;

		//Triangles will be build accessing this vertice pool
		vector<vec3> verts;

		//The file will be read
		//It should behave like a text file
		//Each line such have its first character as an identifier
		while (!f.eof()) {
			char line[128];
			f.getline(line, 128);

			strstream s;
			s << line;

			//Store the identifier
			char junk;

			//'v' is the identifier for vertices
			if (line[0] == 'v') {
				vec3 v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			else if (line[0] == 'f') {
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				//Since OBJ files start counting from 1 it must be subtracted from the index
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
			
		}

		return true;
	}
};
struct mat4x4 {
	float m[4][4] = { 0 };
};

class engine3D : public olcEngine {
public:
    engine3D() {
        m_sAppName = L"Rendering Demo";
		vectorCamera = { 0.0f,0.0f,0.0f };
		lightDir = { 0.0f,0.0f,1.0f };
		rotationYaw = 0;
    }
private:
    mesh mesh_cube;
	mat4x4 matProj;
	float fTheta;
	vec3 lightDir;
	vec3 vectorCamera;	 
	vec3 vectorCameraLooking;
	float rotationYaw;

	

#pragma region Vector Operations


	//& before a varible means acessing its space in memory and not getting a new instance of it
	vec3 VectorAdd(vec3 &v1, vec3 &v2) {
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}
	vec3 VectorSub(vec3 &v1, vec3 &v2) {
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}
	vec3 VectorMul(vec3  &v1, float n) {
		return { v1.x * n, v1.y * n, v1.z * n };
	}
	vec3 VectorDiv(vec3 &v1, float n) {
		return { v1.x / n, v1.y / n, v1.z / n };
	}
	float VectorDot(vec3 &v1, vec3 &v2) {
		return  v1.x * v2.x + v1.y * v2.y + v1.z * v2.z ;
	}
	vec3 VectorCross(vec3 &v1, vec3 &v2) {
		vec3 v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}
	float VectorLen(vec3 &v1) {
		return sqrt(VectorDot(v1,v1));
	}
	vec3 VectorNormalize(vec3 &v1) {
		float l = VectorLen(v1);
		return { v1.x / l, v1.y / l, v1.z / l };
	}
#pragma endregion
#pragma region Matrix Operations

	vec3 MatrixMultiplyVector (mat4x4 &m, vec3 &i) {
		vec3 o;
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		o.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		return o;
	}
	mat4x4 MatrixIdentity() {
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}
	mat4x4 MatrixCreateRotationX(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[1][2] = sinf(fAngleRad);
		matrix.m[2][1] = -sinf(fAngleRad);
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 MatrixCreateRotationY(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][2] = sinf(fAngleRad);
		matrix.m[2][0] = -sinf(fAngleRad);
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 MatrixCreateRotationZ(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][1] = sinf(fAngleRad);
		matrix.m[1][0] = -sinf(fAngleRad);
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 MatrixCreateTranslation(float x, float y, float z)
	{
		//mat4x4 matrix = MatrixIdentity();
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	mat4x4 MatrixCreateProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
	{
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}

	mat4x4 MatrixMultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		return matrix;
	}

	mat4x4 MatrixPointAt(vec3 &pos, vec3 &target, vec3 &up)
	{
		// Calculate new forward direction
		vec3 newForward = VectorSub(target, pos);
		newForward = VectorNormalize(newForward);

		// Calculate new Up direction
		vec3 a = VectorMul(newForward, VectorDot(up, newForward));
		vec3 newUp = VectorSub(up, a);
		newUp = VectorNormalize(newUp);

		// New Right direction is easy, its just cross product
		vec3 newRight = VectorCross(newUp, newForward);

		// Construct Dimensioning and Translation Matrix	
		mat4x4 matrix;
		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;

	}

	mat4x4 MatrixQuickInverse(mat4x4& m) // Only for Rotation/Translation Matrices
	{
		mat4x4 matrix;
		matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}



#pragma endregion

	//This function is responsible to find the right "shade" 
	//for a color given its luminosity
	wchar_t GetShade(float p) {
		wchar_t r = NAN;
		//Apply p in a quadratic function to smooth out the lighting change
		//p =  (p * p) + 0*p +  1.0f;
		p = (p * p) + 0.1f;
		int s = (int)(p * 4);
		switch (s)
		{
		case 0: r = PIXEL_QUARTER; break;
		case 1: r = PIXEL_HALF; break;
		case 2: r = PIXEL_THREEQUARTERS; break;
		case 3: r = PIXEL_SOLID; break;
		default:
			r = PIXEL_SOLID;
		}
		return (r);
	}
	void InputCameraMovement(float fElapsedTime) {
		//Y Axis
		if (GetKey(VK_UP).bHeld)
			vectorCamera.y -= 8.0f * fElapsedTime;
		if (GetKey(VK_DOWN).bHeld)
			vectorCamera.y += 8.0f * fElapsedTime;

		//X Axis
		if (GetKey(VK_RIGHT).bHeld)
			vectorCamera.x += 8.0f * fElapsedTime;
		if (GetKey(VK_LEFT).bHeld)
			vectorCamera.x -= 8.0f * fElapsedTime;

		vec3 vectorForward = VectorMul(vectorCameraLooking, 8.0f * fElapsedTime);

		//Foward Axis
		if (GetKey(L'W').bHeld)
			vectorCamera = VectorAdd(vectorCamera, vectorForward);
		if (GetKey(L'S').bHeld)
			vectorCamera = VectorSub(vectorCamera, vectorForward);

		//Rotate along Y axis
		if (GetKey(L'D').bHeld)
			rotationYaw -= 2.0f * fElapsedTime;
		if (GetKey(L'A').bHeld)
			rotationYaw += 2.0f * fElapsedTime;
	}
public:
    bool OnUserCreate() override {

		//mesh_cube.LoadObjectFile("VideoShip.obj");
		mesh_cube.LoadObjectFile("axis.obj");

		matProj = MatrixCreateProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

		return true;
    }
    bool OnUserUpdate(float fElapsedTime) override {
		InputCameraMovement(fElapsedTime);
		//Clean screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Set up rotation matrices
		mat4x4 matrixRotZ, matrixRotX;
		//fTheta += 1.0f * fElapsedTime;

		// Rotations
		matrixRotZ = MatrixCreateRotationZ(fTheta*0.5f);
		matrixRotX = MatrixCreateRotationX(fTheta);

		mat4x4 matrixTransform = MatrixCreateTranslation(0.0f, 0.0f, 16.0f);

		mat4x4 matrixWorld = MatrixIdentity();
		matrixWorld = MatrixMultiplyMatrix(matrixRotZ, matrixRotX);
		matrixWorld = MatrixMultiplyMatrix(matrixWorld, matrixTransform);

		vectorCameraLooking = { 0,0,1 };
		vec3 vectorUp = { 0,1,0 };
		vec3 vectorTarget = { 0,0,1 };
		mat4x4 matrixCameraRotation = MatrixCreateRotationY(rotationYaw);
		vectorCameraLooking = MatrixMultiplyVector(matrixCameraRotation, vectorTarget);
		vectorTarget = VectorAdd(vectorCamera, vectorCameraLooking);

		//Make view matrix from camera position, its target and the Y axis
		mat4x4 matrixCamera = MatrixPointAt(vectorCamera, vectorTarget, vectorUp);
		mat4x4 matrixView = MatrixQuickInverse(matrixCamera);

		vector<triangle> trianglesToRasterize;

		//Draw triangles from cube
		for (auto tri : mesh_cube.tris)
		{
			triangle triProjected, triTransformed, triViewed;

			// Rotate in Z-Axis
			triTransformed.vert[0] = MatrixMultiplyVector(matrixWorld, tri.vert[0]);
			triTransformed.vert[1] = MatrixMultiplyVector(matrixWorld, tri.vert[1]);
			triTransformed.vert[2] = MatrixMultiplyVector(matrixWorld, tri.vert[2]);

			//Calculate the triangule normal
			//Using the cross product of its catheters
			//Order of vertices matter to accuratelly get the catheters
			//That is why all the vertices have a set order of assignment
			vec3 normal, line1, line2;
			line1 = VectorSub(triTransformed.vert[1], triTransformed.vert[0]);
			line2 = VectorSub(triTransformed.vert[2], triTransformed.vert[0]);

			normal = VectorCross(line1, line2);
			//It is normally normal to normalize normals
			normal = VectorNormalize(normal);

			// Get Ray from triangle to camera
			vec3 cameraRay = VectorSub(triTransformed.vert[0], vectorCamera);

			// If ray is aligned with normal, then triangle is visible
			if (VectorDot(normal, cameraRay) < 0.0f)
			{
				//Lighting
				float length = sqrtf(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
				float lightDot = VectorDot(normal, lightDir);
				triProjected.light_incidence = lightDot;

				// World Space --> View Space
				triViewed.vert[0] = MatrixMultiplyVector(matrixView, triTransformed.vert[0]);
				triViewed.vert[1] = MatrixMultiplyVector(matrixView, triTransformed.vert[1]);
				triViewed.vert[2] = MatrixMultiplyVector(matrixView, triTransformed.vert[2]);
					 
				//Project triangles from 3D --> 2D
				//Using the set projection matrix
				triProjected.vert[0] = MatrixMultiplyVector(matProj, triViewed.vert[0]);
				triProjected.vert[1] = MatrixMultiplyVector(matProj, triViewed.vert[1]);
				triProjected.vert[2] = MatrixMultiplyVector(matProj, triViewed.vert[2]);

				triProjected.vert[0] = VectorDiv(triProjected.vert[0], triProjected.vert[0].w);
				triProjected.vert[1] = VectorDiv(triProjected.vert[1], triProjected.vert[1].w);
				triProjected.vert[2] = VectorDiv(triProjected.vert[2], triProjected.vert[2].w);

				//Scale into view
				vec3 offsetView = { 1,1,0 };
				triProjected.vert[0] = VectorAdd(triProjected.vert[0], offsetView);
				triProjected.vert[1] = VectorAdd(triProjected.vert[1], offsetView);
				triProjected.vert[2] = VectorAdd(triProjected.vert[2], offsetView);

				triProjected.vert[0].x *= 0.5f * (float)ScreenWidth();
				triProjected.vert[0].y *= 0.5f * (float)ScreenHeight();
				triProjected.vert[1].x *= 0.5f * (float)ScreenWidth();
				triProjected.vert[1].y *= 0.5f * (float)ScreenHeight();
				triProjected.vert[2].x *= 0.5f * (float)ScreenWidth();
				triProjected.vert[2].y *= 0.5f * (float)ScreenHeight();

				trianglesToRasterize.push_back(triProjected);
			}
			
		}
	
		//Before drawing the triangles they must be sort in order to avoid faces overlapping
		sort(trianglesToRasterize.begin(), trianglesToRasterize.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.vert[0].z + t1.vert[1].z + t1.vert[2].z)/3;
				float z2 = (t2.vert[0].z + t2.vert[1].z + t2.vert[2].z)/3;
				return z1 < z2;
			});

		for (auto& triProjected : trianglesToRasterize)
		{
			FillTriangle(triProjected.vert[0].x, triProjected.vert[0].y,
				triProjected.vert[1].x, triProjected.vert[1].y,
				triProjected.vert[2].x, triProjected.vert[2].y,
				GetShade(triProjected.light_incidence), FG_RED);
		}

		engine3D s;
        return true;
    }
};

int main()
{
    engine3D demo;

    if (demo.ConstructConsole(256, 240, 2, 2));
        demo.Start();

    return(0);
}