#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <vector>
#include <cmath>
#include <time.h>
#include <map>

/// TO DO LIST
/// should make it so screen is not refreshed every frame
/// OPTIMISE - if direction = {0,0}, they will have duplicated body parts
/// OPTIMISE - Grid is being drawn to cover a bug in the DrawSnakes function where it draws too much on far left.
/// 
/// idea - bomb u can drop which will explode some body. 
/// 
/// change head to triangle
/// 
/// add extended menu + rules for noobs
/// 
/// add menu  - buttons for menu are done. Make play button to switch game mode + set AIs.

/// SNAKE BODY DATA STRUCTURE
/// Snake body is no longer stored
/// only their head is stored
/// their body is stored in an olc::sprite called board as olc::pixels

enum GameState {
	MENU, MENU_CONTROLS, RULES , BEFOREPLAY, DURINGPLAY, ENDPLAY
};

struct snake {
	snake(olc::Pixel col, const olc::vi2d& Head, std::vector<olc::Key> con) : colour(col), controls(con) {
		head = Head;
	}
	olc::vi2d direction = olc::vi2d(0,0);
	olc::vi2d head;
	std::list<olc::Key> keyCache;
	int points = 0;
	bool dead = false;
	bool startedMoving = false; 
	bool isCPU = false;
	olc::Pixel colour;
	const std::vector<olc::Key> controls; // order is: UP, RIGHT, DOWN, LEFT , const

	void updateControls(bool keyStates[4], const int size = 4) {
		for (int i = 0; i < size; i++) {
			if (keyStates[i]) { // if key is pressed
				// try and add command to keyCache;
				if (keyCache.empty()) keyCache.push_front(controls[i]); // no need to for same adjacent commands
				else if (keyCache.front() != controls[i]) keyCache.push_front(controls[i]); // stop double inputs of the same control
			}
		}
	}
	void updateDirection() {
		if (keyCache.empty()) return;
		const auto& key = keyCache.back();
		if (key == controls[0]) { // change direction to up
			direction = olc::vi2d(0, -1); // 0 in x, 1 in y
		}
		else if (key == controls[1]) { // change direction to right
			direction = olc::vi2d(1, 0);
		}
		else if (key == controls[2]) { // change direction to down
			direction = olc::vi2d(0, 1);
		}
		else if (key == controls[3]) { // change direction to left
			direction = olc::vi2d(-1, 0);
		}
		keyCache.pop_back();
	}
};

class BMTron : public olc::PixelGameEngine
{
public:
	BMTron()
	{
		sAppName = "BMTron";
	}
private:
	float round(float a) {
		if (a - floor(a) > 0.5f) {
			return a + 1;
		}
		else {
			return a;
		}
	}
public:
private:
	void testUpdateControls() {
		bool controls[] = { 0,0,0,1 };
		players[0].updateControls(controls);
		for (const auto& i : players[0].keyCache) {
			std::cout << i << std::endl;
		}
		bool controls1[] = { 0,1,0,0 };
		players[0].updateControls(controls1);
		for (const auto& i : players[0].keyCache) {
			std::cout << i << std::endl;
		}
		bool controls2[] = { 1,0,0,0 };
		players[0].updateControls(controls2);
		for (const auto& i : players[0].keyCache) {
			std::cout << i << std::endl;
		}

	}
	bool OnUserCreate() override
	{
		srand(std::time(NULL));
		tileWidth = (int)((ScreenWidth() - offsetX) / numTiles - 1);
		olc::vi2d startingPos[4];
		startingPoses(startingPos);
		players.push_back(snake(olc::RED, startingPos[0], { olc::W, olc::D, olc::S, olc::A }));
		players.push_back(snake(olc::BLUE, startingPos[1], { olc::Y, olc::J, olc::H, olc::G }));
		players.push_back(snake(olc::YELLOW, startingPos[2], { olc::P, olc::OEM_3, olc::OEM_1, olc::L })); // this is wrong
		players.push_back(snake(olc::GREEN, startingPos[3], { olc::UP, olc::RIGHT, olc::DOWN, olc::LEFT }));
		
		players[1].isCPU = true;
		players[2].isCPU = true;
		players[3].isCPU = true;

		olc::vi2d directions[4] = {
			{0, -1},
			{1, 0},
			{0, 1},
			{-1, 0}
		}; 
		
		players[0].direction = olc::vi2d(0, 0); // these were all set to something
		players[1].direction = olc::vi2d(0, 0);
		players[2].direction = olc::vi2d(0, 0);
		players[3].direction = olc::vi2d(0, 0);

		
		//DrawGrid();
		//DrawSnakes();
		//DrawPoints();
		

		//testUpdateControls();
		
		return true;
	}
	std::vector<snake> players = {};
	const int numTiles = 40;
	olc::Sprite board{ numTiles, numTiles }; // olc::BLACK is being used as default;
	const int offsetX = 0;
	const int offsetY = 40;
	int tileWidth;
	float time = 0;
	float secondsPerSquare = 0.09f;
	bool startedPlay = false;
	GameState gamestate = MENU;

	void startingPoses(olc::vi2d pos[4]) {
		// these come from the equation 2a + b + 2 = n where are is the number of tile left of RED and b is the tile between RED and BLUE
		float a = round((numTiles - 2) / 3);
		float b = numTiles - 2 - 2 * a;
		pos[0] = olc::vi2d(a, a);
		pos[1] = olc::vi2d(numTiles - 1 - a, a);
		pos[2] = olc::vi2d(a, numTiles - 1 - a);
		pos[3] = olc::vi2d(numTiles - 1 - a, numTiles - 1 - a);
		/*std::cout << "starting positions are \n";
		for (int i = 0; i < 4; i++) {
			std::cout << pos[i].x << " " << pos[i].y << std::endl;
		}*/
	}
	void DrawGrid() {
		// vertical lines
		for (float x = 0.0f; x <= ScreenWidth(); x += ((float)(ScreenWidth() - 1) / numTiles)) {
			int xTemp = (int)x;
			DrawLine(xTemp + offsetX, 0 + offsetY, xTemp + offsetX, ScreenWidth() + offsetY - 1, olc::DARK_GREY);
		}
		// horizontal lines
		for (float y = 0.0f; y <= ScreenWidth(); y += ((float)(ScreenWidth() - 1) / numTiles)) {
			int yTemp = (int)y;
			DrawLine(0 + offsetX, yTemp + offsetY, ScreenWidth() + offsetX, yTemp + offsetY, olc::DARK_GREY);
		}
	}
	void DrawSnakes() {
		for (int x = 0; x < numTiles; x++) {
			for (int y = 0; y < numTiles; y++) {
				olc::Pixel temp = board.GetPixel(x, y);
				if (temp != olc::BLACK) {
					FillRect(coordToPixel(x) + offsetX, coordToPixel(y) + offsetY, tileWidth, tileWidth, temp);
				}
			}
		}
	}
	void DrawPoints() {
		// clear the banner
		// draw the text
		FillRect(0, 0, ScreenWidth(), offsetY, olc::BLACK);
		for (int i = 0; i < 4; i++) {
			olc::vi2d size = GetTextSizeProp(std::to_string(players[i].points));

			DrawStringProp((ScreenWidth() * (i+1)) / 5 - size.x, offsetY / 2 - size.y, std::to_string(players[i].points), players[i].colour, ( 3 * offsetY / 5) / size.y);
		}
	}
	void DrawTitle() {
		int x = ScreenWidth() / 5; // 3/5 of x screen is the title
		int y = ScreenHeight() / 12;
		olc::vi2d size = GetTextSizeProp("BMTron");
		// scalefactor = image size / actual size
		//float scaleFac
		//DrawStringProp(x - (), y)
		DrawStringProp(x, y, "BMTron", olc::RED, (ScreenWidth() * 3 / 5) / size.x);
		olc::vi2d centre = olc::vi2d(ScreenWidth() / 2, ScreenHeight() / 4);
		olc::vi2d textSize = GetTextSizeProp("Green For AI, White For Human");
		DrawStringProp(centre.x - textSize.x * 2 / 2, centre.y - textSize.y * 2 / 2 - textSize.y - 20, "Green For AI, White For Human", olc::RED, 2);
		textSize = GetTextSizeProp("Click Middle Buttons To Swap");
		DrawStringProp(centre.x - textSize.x * 2 / 2, centre.y - textSize.y * 2 / 2 , "Click Right Buttons To Swap", olc::RED, 2);

	}
	enum buttonState {
		NOTHING, HOVER, CLICKED
	};
	buttonState buttons[5] = { NOTHING, CLICKED, CLICKED, CLICKED, NOTHING};
	//buttonState playButton = NOTHING;
	olc::vi2d getButtonCoord(int i) {
		int x = ScreenWidth() / 4;
		int size = 2 * ScreenHeight() / 27;
		int y = ScreenHeight() / 4 + (i * 2 + 1) * size;
		return { x,y };
	}
	void DrawControls() {
		std::string controls[4] = { "WASD", "YGHJ", "PL;'", "ARROWS" };
		for (int i = 0; i < 4; i++) {
			olc::vi2d coord = getButtonCoord(i);
			coord.x = ScreenWidth() / 4 * 3;
			olc::vi2d textSize = GetTextSizeProp(controls[i]) * 2;
			DrawStringProp(coord - textSize / 2, controls[i], players[i].colour, 2);
		}
	}
	void DrawMenu() {
		int size = 2 * ScreenHeight() / 27;
		for (int i = 0; i < 4; i++) {
			olc::vi2d coord = getButtonCoord(i);
			FillRect(coord.x - size / 2, coord.y - size / 2, size, size, players[i].colour);
			coord.x *= 2;
			olc::vf2d textSize = GetTextSizeProp("AI");
			coord.x += size / 5;
			coord.y += size / 5;
			float scaleFactor = size / (textSize.x + 5);
			DrawStringProp(coord.x - size / 2, coord.y - size / 2 + (textSize.y * scaleFactor / 2), "AI", players[i].colour, scaleFactor);
			coord.x -= size / 5;
			coord.y -= size / 5;
			switch (buttons[i]) {
			case NOTHING:
				DrawRect(coord.x - size / 2, coord.y - size / 2, size, size);
				break;
			case HOVER:
				DrawRect(coord.x - size / 2, coord.y - size / 2, size, size, olc::YELLOW);
				break;
			case CLICKED:
				DrawRect(coord.x - size / 2, coord.y - size / 2, size, size, olc::GREEN);
				break;
			}

			// Draw Play button
			
			textSize = GetTextSizeProp("PLAY");
			scaleFactor = 2;
			int breathingRoom = 8; // pixels between PLAY and edge of button

			olc::vf2d centre = getButtonCoord(4);
			centre.x = ScreenWidth() / 2;

			DrawStringProp(centre.x - textSize.x * scaleFactor / 2 + 1, centre.y - textSize.y * scaleFactor / 2 + 1, "PLAY", olc::GREEN, 2);
			olc::vf2d topRight = centre - textSize * scaleFactor / 2 - olc::vf2d(breathingRoom, breathingRoom);
			olc::vf2d buttonSize = textSize * olc::vf2d(scaleFactor, scaleFactor) + 2 * olc::vf2d(breathingRoom, breathingRoom) - olc::vf2d(2,2);
			switch (buttons[4]) {
			case NOTHING:
				DrawRect(topRight, buttonSize);
				break;
			case HOVER:
				DrawRect(topRight, buttonSize, olc::YELLOW);
				break;
			case CLICKED:
				DrawRect(topRight, buttonSize, olc::GREEN);
				break;
			}
			DrawControls();
		}
	}
	bool coordInSquare(olc::vi2d coord, olc::vi2d squarePos, olc::vf2d size) {
		return coord.x > squarePos.x && coord.x < squarePos.x + size.x &&
			coord.y > squarePos.y && coord.y < squarePos.y + size.y;
	}
	void updateButtons() {
		olc::vf2d size = olc::vf2d(2 * ScreenHeight() / 27, 2 * ScreenHeight() / 27);
		for (int i = 0; i < 5; i++) {
			olc::vf2d squarePos;
			if (i == 4) {
				olc::vf2d textSize = GetTextSizeProp("PLAY");
				int scaleFactor = 2;
				int breathingRoom = 8; // pixels between PLAY and edge of button

				olc::vf2d centre = getButtonCoord(4);
				centre.x = ScreenWidth() / 2;

				squarePos = centre - textSize * scaleFactor / 2 - olc::vf2d(breathingRoom, breathingRoom);
				size = textSize * olc::vf2d(scaleFactor, scaleFactor) + 2 * olc::vf2d(breathingRoom, breathingRoom) - olc::vf2d(2, 2);
			}
			else {
				squarePos = getButtonCoord(i);
				squarePos.x *= 2;
				squarePos -= size / 2;
			}
			if (coordInSquare(GetMousePos(), squarePos , size)) {
				
				if (GetMouse(0).bReleased) {
					if (buttons[i] == CLICKED) buttons[i] = HOVER;
					else buttons[i] = CLICKED;
				}
				else {
					if (buttons[i] == NOTHING) buttons[i] = HOVER;
					// if hovering and clicked - stay clicked - make table
					//buttons[i] = HOVER;
				}
			}
			else if (buttons[i] == HOVER) {
				buttons[i] = NOTHING;
				// not in square + HOVER
			}

		}
	}
	float coordToPixel(int coord) {
		return coord * (ScreenWidth() - 1) / numTiles + 1;
	}
	bool checkGameOver() {
		// returns true if game over: if all players are dead
		int numDead = 0;
		for (const snake& player : players) {
			if (player.dead) numDead++;
		}
		return numDead == 3 || numDead == 4;
	}
	void MoveSnakes() {
		// calculate target pos - calculate all target posses
		std::vector<olc::vi2d> targets; 
		std::vector<bool> collided;
		for (auto& player : players) {
			if (!player.dead) {
				targets.push_back(player.head + player.direction);
				collided.push_back(false);
			}
			// if dead: they wont be considered
		}

		// check for collision
		CheckCollision(targets, collided, targets.size()); // size is passed in because dead players will not be checked
		// resolve collision - kill'em if collided
		// check this function it may not work
		int collidedIndex = 0;
		for (int i = 0; i < 4; i++) {
			if (players[i].dead) continue; // if dead before this set of collisions
			else {
				if (collided[collidedIndex]) {
					collidedIndex++;
					// kill'em
					players[i].dead = true;
				}
				else {
					// add target to board
					// also change head to new target.
					players[i].head = targets[collidedIndex];
					board.SetPixel(targets[collidedIndex], players[i].colour);
					collidedIndex++;
				}
			}
			
		}
	}
	bool posCollidedWall(const olc::vi2d& pos) {
		if (pos.x < 0 || pos.x > numTiles || pos.y < 0 || pos.y > numTiles) return true;
		return false;
	}

	struct Pos {
		olc::vi2d target;
		int index;
	};
	bool inVec(const std::vector<Pos>& container, const olc::vi2d item, int& index) {
		for (int i = 0; i < (signed int)container.size(); i++) {
			index = i;
			if (container[i].target == item) return true;
		}
		return false;
	}
	void CheckCollision(const std::vector<olc::vi2d>& targets, std::vector<bool>& collided, int numAlive) {
		// boundry checks
		
		// RULE DO NOT MAKE PLAYERS DEAD HERE: players[n].dead = true IS BANNED IN THIS FUNCTION

		// check against other heads:
		if (numAlive == 0) return;
		std::vector<Pos> unique = { {targets[0], 0} };
		for (int i = 1; i < numAlive; i++) {
			int index = 0;
			if (inVec(unique, targets[i], index)) {
				collided[i] = true;
				collided[index] = true; 
			}
			else {
				unique.push_back({ targets[i], i });
			}
		}
		// check against the boundary
		for (int i = 0; i < numAlive; i++) {
			if (!collided[i]) {
				if (targets[i].x < 0 || targets[i].x > numTiles || targets[i].y < 0 || targets[i].y > numTiles) {
					collided[i] = true;
				}
			}
		}
		// checks against body parts - which is in the board sprite
		for (int target = 0; target < numAlive; target++) {
			if (collided[target]) continue; // if mover is dead: dont bother.
			if (board.GetPixel(targets[target]) != olc::BLACK) {
				// collided with some colour. Doesnt matter which one
				collided[target] = true;
			}
		}
		
	}
	bool stopPlay = false;

	void UpdatePlayerControls() {
		// update controls
		
		for (int i = 0; i < 4; i++) {
			if (players[i].isCPU) continue;
			bool states[] = { GetKey(players[i].controls[0]).bPressed, GetKey(players[i].controls[1]).bPressed,
				GetKey(players[i].controls[2]).bPressed, GetKey(players[i].controls[3]).bPressed };
			for (int j = 0; j < 4; j++) {
				if (states[j]) {
					players[i].startedMoving = true;
				}
			}
			
			players[i].updateControls(states);
			
		}
	}
	// create a function to calculate the number of free spaces in a direction from a particular starting point
	// this will be used by the AIs to not go into a trap
	int ConsecutiveFreeSpaces(olc::vi2d pos, olc::vi2d direction) {
		int frees = 0;
		while (board.GetPixel(pos + direction) == olc::BLACK || posCollidedWall(pos)) {
			frees++;
			pos += direction;
		}
		return frees;
	}
	void PlayerAI(bool states[4], int playerIndex) {
		// calculate the 3 positions
		// check collision for each of them
		// randomly pick one of the non-collision ones
		snake& player = players[playerIndex];
		std::vector<olc::vi2d> options; // array of 3 potential positions
		std::map<olc::vi2d, olc::vi2d> rotateLeft{
			{{0,-1}, {-1, 0}},
			{{1,0}, {0,-1}},
			{{0,1}, {1,0}},
			{{-1, 0}, {0, 1}}
		};
		options.push_back(rotateLeft[player.direction] + player.head);
		options.push_back(player.direction + player.head);
		// to rotate right, simply multiply the left vector by -1
		options.push_back((rotateLeft[player.direction] * -1) + player.head);
		std::vector<bool> collided = {0,0,0}; // represents whether the options cause collisions
		for (int i = 0; i < 3; i++) {
			if (posCollidedWall(options.at(i))) {
				// collided
				collided[i] = true;
			}
		}
		// collisions with snakes on board
		for (int i = 0; i < 3; i++) {
			if (!collided[i]) {
				// only check further collisions if they haven't collided yet.
				if (board.GetPixel(options.at(i)) != olc::BLACK) {
					// collided
					collided[i] = true;
				}
			}
		}
		// remove posses that have collided
		int size = 3;
		for (int i = 0; i < size; i++) {
			if (collided[i]) {
				options.erase(options.begin() + i);
				collided.erase(collided.begin() + i);
				size--;
				i--;
			}
		}
		std::map<olc::vi2d, int> dirToIndex{
			{{0,-1}, 0},
			{{1,0}, 1},
			{{0,1}, 2},
			{{-1, 0}, 3}
		};
		if (options.empty()) {
			for (int i = 0; i < 4; i++) {
				states[i] = 0;
			}
		}
		else {
			for (const auto& option : options) {
				if (option - player.head == player.direction) {
					if (rand() % 4 != 0) {
						return;
					}
					else {
						break;
					}
				}
			}
			olc::vi2d chosen;
			std::vector<int> numFrees;
			for (const auto& option : options) {
				numFrees.push_back(ConsecutiveFreeSpaces(player.head, option - player.head));
			}
			int biggestIndex = 0;
			for (int i = 0; i < numFrees.size(); i++) {
				if (numFrees[i] > numFrees[biggestIndex]) {
					biggestIndex = i;
				}
			}
			chosen = options[biggestIndex];
			if (rand() % 8 == 0) {
				int x = rand() % options.size();
				olc::vi2d chosen = options[x];
			}
			chosen -= player.head; // this gives the desired direction
			player.direction = chosen;
			states[dirToIndex[chosen]] = 1;
		}
		// maybe check the options 5 blocks down each direction??
	}
	
	void perFrame(float& time) {
		bool allMoving;
		switch (gamestate) {
		case MENU:
			updateButtons();
			DrawTitle();
			DrawMenu();
			if (buttons[4] == CLICKED) {
				// transition to BEFOREPLAY
				for (int i = 0; i < 4; i++) {
					if (buttons[i] == CLICKED) {
						players[i].isCPU = true;
					}
					else {
						players[i].isCPU = false;
					}
				}
				Clear(olc::BLACK);

				olc::vi2d directions[4] = {
					{0, -1},
					{1, 0},
					{0, 1},
					{-1, 0}
				};

				for (snake& player : players) {
					if (player.isCPU) {
						player.startedMoving = true;
						player.direction = directions[rand() % 4];
					}
				}
				// initialize board
				for (int i = 0; i < numTiles; i++) {
					for (int j = 0; j < numTiles; j++) {
						board.SetPixel(i, j, olc::BLACK);
					}
				}
				for (const snake& player : players) {
					board.SetPixel(player.head, player.colour);
				}
				gamestate = BEFOREPLAY;
			}
			break;
		case BEFOREPLAY:
			// check if everyone has pressed or not
			// if they have, change state to DURINGPLAY
			DrawGrid();
			DrawSnakes();
			DrawPoints();
			UpdatePlayerControls();
			allMoving = true;
			for (const snake& player : players) {
				if (!player.startedMoving) {
					allMoving = false;
				}
			}
			if (allMoving) {
				gamestate = DURINGPLAY;
				for (snake& player : players) {
					player.updateDirection();
				}
			}
			break;
			
		case DURINGPLAY:
			if (time > secondsPerSquare) {
				if (checkGameOver()) {
					gamestate = ENDPLAY;
					std::cout << "THE GMAE IS OVER\n";
					for (snake& player : players) {
						if (!player.dead) player.points++;
					}
					DrawPoints();
					break;
				}
				for (int i = 0; i < 4; i++) {
					if (players[i].isCPU) {
						bool states[] = { 0,0,0,0 };
						PlayerAI(states, i);
						players[i].updateControls(states);
					}
				}
				for (snake& player : players) {
					player.updateDirection();
				}
				time = 0;
				MoveSnakes();
			}
			DrawSnakes();
			DrawGrid();
			break;
			// do stuff
			// check if game is over
			// if game over, change state to ENDPLAY
		case ENDPLAY:
			Reset();
			std::cout << "RESET\n";
			if (GetKey(olc::ENTER).bPressed) {
				gamestate = BEFOREPLAY;
				Clear(olc::BLACK);
				DrawGrid();
				DrawSnakes();
				DrawPoints();
			}
			break;
		}
	}
	void Reset() {
		
		olc::vi2d directions[4] = {
			{0, -1},
			{1, 0},
			{0, 1},
			{-1, 0}
		};

		// recalculate starting positions
		olc::vi2d startingPos[4];
		startingPoses(startingPos);
		// reset heads for players
		for (int i = 0; i < 4; i++) {
			players[i].head = startingPos[i];
		}
		// reset board
		for (int i = 0; i < numTiles; i++) {
			for (int j = 0; j < numTiles; j++) {
				board.SetPixel(i, j, olc::BLACK);
			}
		}
		for (const snake& player : players) {
			board.SetPixel(player.head, player.colour);
		}
		// reset keyCaches for players
		for (snake& player : players) {

			player.keyCache.clear();
			player.direction = { 0,0 };
			player.dead = false;
			player.startedMoving = false;
			if (player.isCPU) {
				player.startedMoving = true;
				player.direction = directions[rand() % 4];
			}
		}
	}
	bool OnUserUpdate(float fElapsedTime) override
	{ /// ; is OEM_1 || ' is OEM_3
		
		time += fElapsedTime;
		//stopPlay = false;
		if (stopPlay) {
			// wait until enter has been pressed
			if (GetKey(olc::ENTER).bPressed) {
				Reset();
			}
			// when pressed, reset
			// set stopPlay to false
		}
		if (gamestate == BEFOREPLAY || gamestate == DURINGPLAY)	UpdatePlayerControls();
		perFrame(time);

		//DrawSnakes();
		//DrawGrid();
		//DrawTitle();
		//DrawMenu();
		return true;
	}
};

int main()
{
	BMTron demo;
	if (demo.Construct(600,640, 1, 1))
		demo.Start();
	return 0;
}