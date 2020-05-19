#include "olcEngine.h";
#include <fstream>;
#include <strstream>;
#include <algorithm>;
 using namespace std;

struct vec3 {
    float x, y, z;
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

			if (line[0] == 'f') {
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
		cameraDir = { 0.0f,0.0f,0.0f };
		lightDir = { 0.0f,0.0f,-1.0f };
    }
private:
    mesh mesh_cube;
	mat4x4 matProj;
	float fTheta;
	vec3 cameraDir;
	vec3 lightDir;

	void MultiplyMatrixVector(vec3& i, vec3& o, mat4x4& m) {
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f)
		{
			o.x /= w; o.y /= w; o.z /= w;
		}
	}
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
public:
    bool OnUserCreate() override {
		//mesh_cube.tris = {

			// SOUTH
			//{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
			//{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

			// EAST                                                      
			//{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
			//{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

			// NORTH                                                     
			//{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
			//{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

			// WEST                                                      
			//{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
			//{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

			// TOP                                                       
			//{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
			//{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

			// BOTTOM                                                    
			//{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
			//{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

		//};

		mesh_cube.LoadObjectFile("VideoShip.obj");

		//Projection Matrix is populated once because the screen dimensions and field 
		//of view will not change in this application
		float fNear = 0.1f;
		float fFar = 1000.0f;
		float fFov = 90.0f;
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
		float fFovRad = 1.0f / tanf(fFov * 0.5f);

		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;

		return true;
    }
    bool OnUserUpdate(float fElapsedTime) override {
		//Clean screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Set up rotation matrices
		mat4x4 matRotZ, matRotX;
		fTheta += 1.0f * fElapsedTime;

		// Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;

		// Rotation X
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(fTheta * 0.5f);
		matRotX.m[1][2] = sinf(fTheta * 0.5f);
		matRotX.m[2][1] = -sinf(fTheta * 0.5f);
		matRotX.m[2][2] = cosf(fTheta * 0.5f);
		matRotX.m[3][3] = 1;

		vector<triangle> trianglesToRasterize;

		//Draw triangles from cube
		for (auto tri : mesh_cube.tris)
		{
			triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

			// Rotate in Z-Axis
			MultiplyMatrixVector(tri.vert[0], triRotatedZ.vert[0], matRotZ);
			MultiplyMatrixVector(tri.vert[1], triRotatedZ.vert[1], matRotZ);
			MultiplyMatrixVector(tri.vert[2], triRotatedZ.vert[2], matRotZ);

			// Rotate in X-Axis
			MultiplyMatrixVector(triRotatedZ.vert[0], triRotatedZX.vert[0], matRotX);
			MultiplyMatrixVector(triRotatedZ.vert[1], triRotatedZX.vert[1], matRotX);
			MultiplyMatrixVector(triRotatedZ.vert[2], triRotatedZX.vert[2], matRotX);

			// Offset into the screen
			triTranslated = triRotatedZX;
			triTranslated.vert[0].z = triRotatedZX.vert[0].z - 8.0f;
			triTranslated.vert[1].z = triRotatedZX.vert[1].z - 8.0f;
			triTranslated.vert[2].z = triRotatedZX.vert[2].z - 8.0f;

			//Calculate the triangule normal
			//Using the cross product of its catheters
			//Order of vertices matter to accuratelly get the catheters
			//That is why all the vertices have a set order of assignment
			vec3 normal, line1, line2;
			line1.x = triTranslated.vert[1].x - triTranslated.vert[0].x;
			line1.y = triTranslated.vert[1].y - triTranslated.vert[0].y;
			line1.z = triTranslated.vert[1].z - triTranslated.vert[0].z;

			line2.x = triTranslated.vert[2].x - triTranslated.vert[0].x;
			line2.y = triTranslated.vert[2].y - triTranslated.vert[0].y;
			line2.y = triTranslated.vert[2].y - triTranslated.vert[0].y;
			line2.z = triTranslated.vert[2].z - triTranslated.vert[0].z;

			//It is normally normal to normalize normals
			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y* line2.x;
			
			float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			
			normal.x /= length;
			normal.y /= length;
			normal.z /= length;

			
			//Only render triangules that are visible
			//For that you need the dot product between
			//The direction vector of the camera and the normal
			//To compare the similarity of the direction between them
			if (normal.x * (triTranslated.vert[0].x - cameraDir.x) +
				normal.y * (triTranslated.vert[0].y - cameraDir.y) +
				normal.z * (triTranslated.vert[0].z - cameraDir.z) < 0.0f) {

				//Lighting
				float length = sqrtf(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
				float lightDot = normal.x * (lightDir.x - cameraDir.x) + 
				normal.y * (lightDir.y - cameraDir.y) + 
				normal.z * (lightDir.z - cameraDir.z);

				triProjected.light_incidence = lightDot;

				//Project triangles from 3D --> 2D
				//Using the set projection matrix
				MultiplyMatrixVector(triTranslated.vert[0], triProjected.vert[0], matProj);
				MultiplyMatrixVector(triTranslated.vert[1], triProjected.vert[1], matProj);
				MultiplyMatrixVector(triTranslated.vert[2], triProjected.vert[2], matProj);

				//Scale into view
				triProjected.vert[0].x += 1.0f; triProjected.vert[0].y += 1.0f;
				triProjected.vert[1].x += 1.0f; triProjected.vert[1].y += 1.0f;
				triProjected.vert[2].x += 1.0f; triProjected.vert[2].y += 1.0f;
				triProjected.vert[0].x *= 0.5f * (float)ScreenWidth();
				triProjected.vert[0].y *= 0.5f * (float)ScreenHeight();
				triProjected.vert[1].x *= 0.5f * (float)ScreenWidth();
				triProjected.vert[1].y *= 0.5f * (float)ScreenHeight();
				triProjected.vert[2].x *= 0.5f * (float)ScreenWidth();
				triProjected.vert[2].y *= 0.5f * (float)ScreenHeight();

				trianglesToRasterize.push_back(triProjected);

				//Rasterize triangle
				//FillTriangle(triProjected.vert[0].x, triProjected.vert[0].y,
				//	triProjected.vert[1].x, triProjected.vert[1].y,
				//	triProjected.vert[2].x, triProjected.vert[2].y,
				//	GetShade(lightDot), FG_RED);

				//Wirefram visualization for debugging
				//DrawTriangle(triProjected.vert[0].x, triProjected.vert[0].y,
				//	triProjected.vert[1].x, triProjected.vert[1].y,
				//	triProjected.vert[2].x, triProjected.vert[2].y,
				//	PIXEL_HALF, FG_DARK_RED);
			}
			
		}
	
		//Before drawing the triangles they must be sort in order to avoid faces overlapping
		sort(trianglesToRasterize.begin(), trianglesToRasterize.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.vert[0].z + t1.vert[1].z + t1.vert[2].z) / 3.0f;
				float z2 = (t2.vert[0].z + t2.vert[1].z + t2.vert[2].z) / 3.0f;
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