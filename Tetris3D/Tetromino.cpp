#include "Tetromino.h"

	Tetromino::Tetromino(std::vector < std::vector<GameSector> >* gf, std::vector <Block> blocks) {
		gamefield = gf;
		width = gamefield->size();
		height = (*gamefield)[0].size();

		createRandomBlock(blocks);
		setupVariables();
	}

	Tetromino::Tetromino(std::vector < std::vector<GameSector> >* gf, std::vector<GameSector> blocksToMoveDown) {
		gamefield = gf;
		width = gamefield->size();
		height = (*gamefield)[0].size();

		createTetrominoFromBlocks(blocksToMoveDown);
		setupVariables();
	}

	//Hier werden die bewegbaren Blöcke also der Tetrominostein gezeichnet. Blöcke die nach einer vollen Reihe nach unten bewegt werden müssen, werden in ein Tetromino verwandelt und nach unten bewegt.
	void Tetromino::draw(float* m_direction, float* r_direction, bool* isMovingDown) {
		if (models.size() == 1) {
			int test = 0;
		}
		moveAboveDown = false;

		//Geht jeden Block des Tetrominosteins durch und zeichent ihn. Durch die Methoden move, movedown und rotate werden die model und transform Matrizes verändert und an die draw Methoden der Blöcke übergeben.
		for (int i = 0; i < models.size(); i++) {
			glm::mat4 model = models[i];
			glm::mat4 transform = transforms[i];
			pivot = elements[pivotIndex].WorldPosition;
			shouldMoveDown = isMovingDown;
			moveDirection = m_direction;

			if (*shouldMoveDown) {
				moveDown(&model, i, shouldMoveDown, m_direction);
			}
			else {
				move(&model, m_direction, i);
				rotate(&transform, r_direction, i);
			}
			if (models.size() > i) {
				models[i] = model;
				transforms[i] = transform;
				elements[i].block.draw(model, transform);
			}
		}

		//Blöcke die nach einer vollen Reihe nach unten bewegt werden müssen, werden hier durch die Methode createTetrominoFromBlocks zu einem Tetromino ungewandelt und direkt nach unten bewegt. 
		if (moveAboveDown) {
			createTetrominoFromBlocks(blocksAbove);
			setupVariables();
			reachedBottom = false;
			*shouldMoveDown = true;
			*moveDirection = -1.0f;
			movingBlocksAboveDown = true;
		}

	}

	//Testet ob links oder rechts platz für eine Bewegung ist.
	bool Tetromino::isMoveable(int xDirection, int yDirection) {
		for (int i = 0; i < currentIndex.size(); i++) {
			int x = currentIndex[i].x + xDirection;
			int y = currentIndex[i].y + yDirection;
			if ((*gamefield)[x][y].blocked) {
				return false;
			}
		}
		return true;
	}

	//Testet ob Rotation in übergebende Richtung möglich ist.
	bool Tetromino::isRotatable(float* r_direction) {
		std::vector <glm::vec2> tempIndex = currentIndex;	
		rotateIndex(-1 * *r_direction, &tempIndex);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				for (int i = 0; i < tempIndex.size(); i++) {
					if ((*gamefield)[x][y].blocked) {
						if (tempIndex[i].x == x && tempIndex[i].y == y) {
							return false;
						}
					}
				}
			}
		}
		return true;
	}

	//Bewegt einen Block nach Links oder Rechts indem an der Model Matrix eine Translation in gegebnder Richtung durchgeführt wird .
	void Tetromino::move(glm::mat4* model, float* m_direction, int index) {
		if (!isMoveable(*m_direction, 0)) {
			*m_direction = 0;
			return;
		}
		if (*m_direction != 0.0f && moved[index] < m_speed) {
			moved[index]++;
			*model = glm::translate(*model, glm::vec3(1.0, 0.0, 0.0) * (1.0f / m_speed) * *m_direction);
		}
		else {
			moved[index] = 0;
			addToIndex((int)*m_direction, false);
			*m_direction = 0.0;
		}
	}

	//Bewegt einen Block nach unten indem an der Model Matrix eine Translation durchgeführt wird. Wenn der Tetrominostein unten ankommt wird er dem Spielfeld hinzugefügt und das Spielfeld wird auf volle Reihen überprüft.
	void Tetromino::moveDown(glm::mat4* model, int index, bool* shouldMoveDown, float* m_direction) {
		bool moveable = isMoveable(0, -1.0f);

		if (movingBlocksAboveDown) {
			d_speed = 30.0f;
		}
		else {
			d_speed = m_speed;
		}

		if (moveable && *shouldMoveDown && movedY[index] < d_speed) {
			movedY[index]++;
			*model = glm::translate(*model, glm::vec3(0.0, 1.0, 0.0) * (1.0f / d_speed) * -1.0f);
		}
		else if (movedY[index] == d_speed) {
			currentIndex[index].y--;
			movedY[index] = 0;
		}
		else if (!moveable) {
			*shouldMoveDown = false;
			*m_direction = 0.0;
			reachedBottom = true;
			addNewTetrominoToGamefield();
			checkForFullRow();
		}
	}

	//Rotiert einen Block in dem die transform matrix rotiert wird.
	void Tetromino::rotate(glm::mat4 *transform, float *r_direction, int index) {

		if ((*r_direction != 0.0f && !isRotating) && !isRotatable(r_direction)) {
			*r_direction = 0.0f;
			return;
		}

		if (*r_direction != 0.0f && rotated[index] < maxAngle) {
			rotated[index] += r_speed;
			glm::vec3 pivotTemp = pivot - elements[index].WorldPosition;
			*transform = glm::translate(*transform, pivotTemp);
			*transform = glm::rotate(*transform, glm::radians(*r_direction * r_speed), glm::vec3(0.0f, 0.0f, 1.0f));
			*transform = glm::translate(*transform, -pivotTemp);
			isRotating = true;
		}
		else {
			rotated[index] = 0.0f;
			rotateIndex(-1 * *r_direction,&currentIndex);
			*r_direction = 0.0f;
		}
	}

	//Rotiert die Indizes des Blockes in übergebende Richtung.
	void Tetromino::rotateIndex(float r_direction, std::vector <glm::vec2> *vector2) {
		if (r_direction == 0.0) {
			isRotating = false;
			return;
		}
		int px = currentIndex[pivotIndex].x;
		int py = currentIndex[pivotIndex].y;
		for (int i = 0; i < currentIndex.size(); i++) {
			int x = currentIndex[i].x;
			int y = currentIndex[i].y;
			int translatedX = x - px;
			int translatedY = y - py;
			int newX = translatedY;
			int newY = -translatedX;
			newX = (newX * r_direction) + px;
			newY = (newY * r_direction) + py;
			(*vector2)[i].x = newX;
			(*vector2)[i].y = newY;
		}
	}

	//erhöht den Index eines Blockes um eins wenn dieser sich nach unten bewegt.
	void Tetromino::addToIndex(int n, bool moveDown) {
		if (n == 0) return;
		for (int i = 0; i < currentIndex.size(); i++) {
			currentIndex[i].x += n * !moveDown;
		}
	}

	//Fügt dem Spielfeld die Blöcke des Tetrominosteins hinzu
	void Tetromino::addNewTetrominoToGamefield() {
		for (int i = 0; i < currentIndex.size(); i++) {
			int x = currentIndex[i].x;
			int y = currentIndex[i].y;
			(*gamefield)[x][y].block = elements[i].block;
			(*gamefield)[x][y].blocked = true;
		}
	}

	//Überprüft ob eine Reihe komplett mit Blöcken gefüllt ist. Falls ja werden die Reihen entfernt und die Blöcke darüber nach unten bewegt.
	void Tetromino::checkForFullRow() {
		int count = 0;
		std::vector <int> numbersY;
		for (int i = 0; i < currentIndex.size(); i++) {
			for (int j = 1; j < width - 1; j++) {
				if ((*gamefield)[j][currentIndex[i].y].blocked) {
					count++;
				}
			}

			if (count == width - 2) {
				numbersY.push_back(currentIndex[i].y);
				for (int j = 1; j < width - 1; j++) {
					GameSector gs((*gamefield)[j][currentIndex[i].y].WorldPosition);
					gs.blocked = false;
					(*gamefield)[j][currentIndex[i].y] = gs;
				}
			}
			count = 0;
		}
		int max = getMaxNumber(numbersY);
		if (max != -1) {
			moveBlocksAboveDown(max);
		}
	}

	//liefert die größte Zahl in dem übergebten integer Vector zurück
	int Tetromino::getMaxNumber(std::vector<int> numbers) {
		if (numbers.size() == 0)
			return -1;
		int max = numbers[0];
		for (int i = 1; i < numbers.size(); i++) {
			max = std::max(max, numbers[i]);
		}
		return max;
	}

	//Fügt dem Vector blocksAbove die Blöcke hinzu die nach unten bewegt werden müssen und entfernt sie aus dem Spielfeld vector. 
	void Tetromino::moveBlocksAboveDown(int max) {
		std::vector<GameSector> blocks;
		for (int i = 1; i < (*gamefield).size()-1; i++) {
			for (int j = max; j < (*gamefield)[0].size()-1; j++) {
				if ((*gamefield)[i][j].blocked) {
					blocks.push_back((*gamefield)[i][j]);
					GameSector gs((*gamefield)[i][j].WorldPosition);
					gs.blocked = false;
					(*gamefield)[i][j] = gs;
				}
			}
		}
		if (blocks.size() != 0) {
			blocksAbove = blocks;
			moveAboveDown = true;
		}
		else {
			reachedBottom = true;
		}
	}

	//Gibt den aktuellen Rotationsgrad zurück
	float Tetromino::getAngle(bool rotate) {
		if (rotate && angleRot <= 90) {
			return angleRot++;
		}
		else if (angleRot >= 90) {

			angleRot = 0;
		}
		else {
			return angleRot;
		}
	}

	//erstellt einen von vier Tetrominosteinen, die Farbe und Form wird zufällig bestimmt
	void Tetromino::createRandomBlock(std::vector <Block> blocks) {
		movingBlocksAboveDown = false;
		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_int_distribution<> colorRange(0, blocks.size() - 1);
		std::uniform_int_distribution<> formRange(0, 3);
		Block blockColor = blocks[colorRange(generator)];
		int randomForm = formRange(generator);

		int x = gamefield->size() / 2;
		int y = (*gamefield)[0].size() - 2;

		switch (randomForm) {
		case 0:
			createLBlock(x, y, blockColor);
			break;
		case 1:
			createSquareBlock(x, y, blockColor);
			break;
		case 2:
			createTBlock(x, y, blockColor);
			break;
		case 3:
			createLongBlock(x, y, blockColor);
			break;
		default:
			break;
		}
	}

	//Positionen des Tetromiosteines wird festgelegt
	void Tetromino::createSingleBlock(int x, int y, Block blockColor) {
		std::vector<glm::vec2> positions = { {x,y} };
		pivotIndex = 0;
		createBlock(blockColor, positions);
	}

	void Tetromino::createLBlock(int x, int y, Block blockColor) {
		std::vector<glm::vec2> positions = { {x,y}, {x,y - 1}, {x, y - 2}, {x + 1,y - 2} };
		pivotIndex = 1;
		createBlock(blockColor, positions);
	}
	void Tetromino::createSquareBlock(int x, int y, Block blockColor) {

		std::vector<glm::vec2> positions = { {x,y}, {x + 1,y}, {x, y - 1}, {x + 1,y - 1} };
		pivotIndex = 0;
		createBlock(blockColor, positions);
	}
	void Tetromino::createTBlock(int x, int y, Block blockColor) {

		std::vector<glm::vec2> positions = { {x,y}, {x - 1,y - 1}, {x, y - 1}, {x + 1,y - 1} };
		pivotIndex = 2;
		createBlock(blockColor, positions);
	}
	void Tetromino::createLongBlock(int x, int y, Block blockColor) {

		std::vector<glm::vec2> positions = { {x,y}, {x,y - 1}, {x, y - 2}, {x,y - 3} };
		pivotIndex = 1;
		createBlock(blockColor, positions);
	}

	void Tetromino::createBlock(Block blockColor, std::vector<glm::vec2> positions) {
		for (int i = 0; i < positions.size(); i++) {
			elements.push_back(GameSector((*gamefield)[positions[i].x][positions[i].y].WorldPosition, blockColor));
		}
	}

	//Initaliziert alle nötigen Variablen für einen neuen Tetrominostein
	void Tetromino::setupVariables() {
		models = std::vector<glm::mat4>();
		transforms = std::vector<glm::mat4>();
		currentIndex = std::vector<glm::vec2>();
		moved = std::vector<int>();
		movedY = std::vector<int>();
		rotated = std::vector<int>();
		for (int i = 0; i < elements.size(); i++) {
			models.push_back(glm::translate(glm::mat4(1.0f), elements[i].WorldPosition));
			transforms.push_back(glm::mat4(1.0f));
			float xPosition = elements[i].WorldPosition.x + (float)width / 2.0f;
			float yPosition = elements[i].WorldPosition.y + (float)height / 2.0f;
			currentIndex.push_back(glm::vec2(xPosition, yPosition));
			moved.push_back(0);
			movedY.push_back(0);
			rotated.push_back(0);
		}
	}

	//Erstellt einen Tetrominostein aus den Blöcken die übergeben worden sind
	void Tetromino::createTetrominoFromBlocks(std::vector<GameSector> blocksToMoveDown) {
		elements = blocksToMoveDown;
		pivotIndex = 0;
	}