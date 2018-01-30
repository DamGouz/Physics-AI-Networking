#include "MazeRenderer.h"
#include <ncltech\CommonUtils.h>

const Vector4 wall_color = Vector4(1.f, 0.8f, 0.3f, 1);



MazeRenderer::MazeRenderer(vector <bool> walls, Mesh* wallmesh)
	: GameObject("")
	, mesh(wallmesh)
	, flat_maze(NULL)
{
	this->SetRender(new RenderNode());
	
	if (!walls.empty())
	{
		uint num_walls = Generate_FlatMaze(walls);

		wall_descriptors.reserve(num_walls);

		Generate_ConstructWalls();

		Generate_BuildRenderNodes();
	}
}

MazeRenderer::~MazeRenderer()
{
	mesh = NULL;

	if (flat_maze)
	{
		delete[] flat_maze;
		flat_maze = NULL;
	}
}

//The search history draws from edges because they already store the 'to'
// and 'from' of GraphNodes.
void MazeRenderer::DrawSearchHistory(vector<pair<int,int>> history, float line_width)
{
	int size = (flat_maze_size + 1) / 3;
	float scale = 2.5 / (3 * (float)size - 1);
	float col_factor = 0.2f / ((float)history.size());

	float index = 0.0f;
	for (int i=0; i<history.size()-1; i+=1)
	{
		Vector3 start = Vector3(
			(history[i].first * 6 + 3 - 3 * size) * scale,
			0.1f,
			(history[i].second * 6 + 3 - 3 * size) * scale);

		Vector3 end = Vector3(
			(history[i+1].first * 6 + 3 - 3 * size) * scale,
			0.1f,
			(history[i + 1].second * 6 + 3 - 3 * size) * scale);

		NCLDebug::DrawThickLine(start, end, line_width, CommonUtils::GenColor(0.8f + index * col_factor));
		index += 1.0f;
	}
}

uint MazeRenderer::Generate_FlatMaze(vector <bool> walls)
{
	//Generates a 3xsize by 3xsize array of booleans, where 
	// a true value corresponds to a solid wall and false to open space.
	// - Each GraphNode is a 2x2 open space with a 1 pixel wall around it.
	uint size = (sqrt(walls.size()) + 1) / 3;

	flat_maze_size = size * 3 - 1;

	if (flat_maze) delete[] flat_maze;
	flat_maze = new bool[flat_maze_size * flat_maze_size];
	memset(flat_maze, 0, flat_maze_size * flat_maze_size * sizeof(bool));
	

	uint base_offset = size * (size - 1);
	uint num_walls = 0;
	//Iterate over each cell in the maze
	for (int i = 0; i < flat_maze_size * flat_maze_size; i++) {
		flat_maze[i] = walls[i];
		if (flat_maze[i]) num_walls++;
	}

	return num_walls;
}

void MazeRenderer::Generate_ConstructWalls()
{
//First try and compact adjacent walls down, so we don't
// just end up creating lots of little cube's.

	//Horizontal wall pass
	for (uint y = 0; y < flat_maze_size; ++y)
	{
		for (uint x = 0; x < flat_maze_size; ++x)
		{
			//Is the current cell a wall?
			if (flat_maze[y*flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				uint old_x = x;

				//If we found a wall, keep iterating in the current
				// search direction and see if we can join it with
				// adjacent walls.
				for (++x; x < flat_maze_size; ++x)
				{
					if (!flat_maze[y * flat_maze_size + x])
						break;

					flat_maze[y * flat_maze_size + x] = false;
				}

				w._xe = x;

				//If the wall is only 1x1, ignore it for the vertical-pass.
				if (w._xe - w._xs > 1)
				{
					flat_maze[y * flat_maze_size + old_x] = false;
					wall_descriptors.push_back(w);
				}
			}
		}
	}

	//Vertical wall pass
	for (uint x = 0; x < flat_maze_size; ++x)
	{
		for (uint y = 0; y < flat_maze_size; ++y)
		{
			if (flat_maze[y * flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				for (++y; y < flat_maze_size && flat_maze[y * flat_maze_size + x]; ++y) {}

				w._ye = y;
				wall_descriptors.push_back(w);
			}
		}
	}



}

void MazeRenderer::Generate_BuildRenderNodes()
{
	//Turn compacted walls into RenderNodes
	RenderNode *cube, *root = Render();

//Turn walls into 3D Cuboids
	const float scalar = 1.f / (float)flat_maze_size;
	for (const WallDescriptor& w : wall_descriptors)
	{
		Vector3 start = Vector3(
			float(w._xs),
			0.0f,
			float(w._ys));

		Vector3 size = Vector3(
			float(w._xe - w._xs),
			0.0f,
			float(w._ye - w._ys)
		);


		start = start * scalar;
		Vector3 end = start + size * scalar;
		end.y = 0.75f;

		Vector3 centre = (end + start) * 0.5f;
		Vector3 halfDims = centre - start;
		halfDims.y /= 2;
		cube = new RenderNode(mesh, wall_color);
		cube->SetTransform(Matrix4::Translation(centre-Vector3(0, (end.y + start.y) * 0.25f,0)) * Matrix4::Scale(halfDims));
		root->AddChild(cube);

	}

//Add bounding edge walls to the maze
	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(-scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(1.f + scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, -scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, 1.f + scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);

	this->SetRender(root);
}