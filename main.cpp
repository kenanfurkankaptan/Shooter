// project1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;
enum WalkDirection {Up, Right, Down, Left};

class Player;
class BulletList;


class Coord{
public:
	float x;
	float y;
	Coord();
	Coord(float x, float y);
};


class Object {
protected:
	sf::RenderWindow* window;
	sf::Texture texture;
	sf::Sprite sprite;
	Coord pos;
	bool isVisible;
public:
	void init(sf::RenderWindow *window, string texturePath, Coord pos);
	void paint();
	sf::Sprite get_sprite();
	Coord& getPosition();
	bool getVisible(void);			//Returns isVisible attribute of barrel
	void setVisible(bool visible);	//Hides or shows the barrel (changes its visible attribute) 
};


class Barrel: public Object {
};

class Sandbag : public Object {	
};

class Bullet : public Object {
	friend class BulletList;
	Bullet* next;
	float speed;			 //Bullet speed (you can pick any speed unit you wish)
	int angle;				//Bullet travel angle
public:
	int player;				//player number that fired it
	void init(Coord pos, sf::RenderWindow *window, int dir, int player_number);
	void move(void);
	void setSpeed(float speed);
};

class BulletList {
	Bullet* list;
	sf::RenderWindow* window;
public:
	BulletList(sf::RenderWindow* window);
	void add(Coord pos, int dir, int player_number);
	void update(void);
	void checkCollision(Player* players, Barrel* barrels, Sandbag* sandbags, int np, int nb, int ns);
};

class Player : public Object {
	sf::Texture textures[14];	//Player texture array (one element per soldier state)
	int state;					//Primary state of the player (range 0-13) (see Figure 5)
	int s;						//Secondary state variable
	bool walkBool;
	int score;					//Score of the player
public:
	void init(sf::RenderWindow *window, string textBasePath, int numTextures, Coord pos);
	void setPosition(Coord pos);
	bool checkCollision(Barrel *barrels, Sandbag *sandbags, int nb, int ns);
	void walk(float speed, int dir, Barrel *barrels, Sandbag *sandbags, int nb, int ns);
	void paint();
	void shoot(BulletList *bullets, int player_num);
	void incrementScore(void);				//Increments score of the player
	int getScore(void);						//Returns the current score of the player 
};



class Game {
	float speed;				//Player walk speed
	int numBarrels;				//Number of barrel objects
	int numSandbags;			//Number of sandbag objects
	int numPlayers;				//Number of player objects
	int width;					//Game screen width
	int height;					//Game screen height
	sf::RenderWindow* window;	//SFML window object
	sf::Texture bgTexture;		//Background tile (grass) texture
	sf::Sprite bgSprite;		//Background tile (grass) sprite
	Barrel *barrels;			//Pointer to barrel objects
	Sandbag *sandbags;			//Pointer to sandbag objects
	Player *players;			//Pointer to player objects
	BulletList *bullets;		//Pointer to BulletList
	sf::Text text1;				//Text object
	sf::Text text2;
	sf::Font font;				//Font object
	int keyboard_input_array[10];
	int player_coord[2] = { 100, 700 };
	void checkScore();
	void reset_game();
public:
	Game(float speed, int w, int h, int nb, int ns, int np);
	void drawBackground();
	void update();
	~Game();
};



Coord::Coord() {							//constructors
	this->x = 0;
	this->y = 0;
};
Coord::Coord(float x, float y) {
	this->x = x;
	this->y = y;
};


void Object::init(sf::RenderWindow *window, string texturePath, Coord pos) {
	this->window = window;
	this->texture.loadFromFile(texturePath);
	this->sprite.setTexture(this->texture);
	this->sprite.setPosition(pos.x, pos.y);
	this->pos = pos;
	this->isVisible = true;
}


void Object::paint() {
	if (isVisible) {
		this->window->draw(this->sprite);
	}
}

sf::Sprite Object::get_sprite() {
	return this->sprite;
}

bool Object::getVisible(void) {
	return this->isVisible;
}
void Object::setVisible(bool visible) {
	this->isVisible = visible;
}

Coord& Object::getPosition() {								//return coords
	return this->pos;
};


Game::Game(float speed, int w, int h, int nb, int ns, int np){				//constructor
	this->speed = speed;
	this->numBarrels = nb;
	this->numSandbags = ns;
	this->numPlayers = np;
	this->width = w;
	this->height = h;
	this-> window = new sf::RenderWindow(sf::VideoMode(width, height), "My Game");	//window pointer
	this->bgTexture.loadFromFile("textures/grass.png");
	this->bgSprite.setTexture(bgTexture);											//set texture for sprite
	bgSprite.setScale(																//set scale to window size
		w / bgSprite.getLocalBounds().width,
		h / bgSprite.getLocalBounds().height);
	this->font.loadFromFile("font/font.ttf");
	this->text1.setFont(font);												//player scores
	this->text2.setFont(font);												//restart text
	
	int barrel_coord[3][2] = { {200, 200}, {500, 300}, {200, 400} };		//initiate barrels in fixed position
	this->barrels = new Barrel[nb];
	for (int i = 0; i < nb; i++) {													
		this->barrels[i].init(this->window, "textures/barrel.png", Coord(barrel_coord[i][0], barrel_coord[i][1]));
	}


	int sandbag_coord[5][2] = { {200, 50}, {500, 100}, {300, 300}, {500, 400}, {400, 550} };//initiate sandbags in fixed position
	this->sandbags = new Sandbag[ns];
	for (int i = 0; i < ns; i++){
		this->sandbags[i].init(this->window, "textures/bags.png", Coord(sandbag_coord[i][0], sandbag_coord[i][1]));
	}

	this->players = new Player[np];										//initiate players in fixed x axis, random y axis
	for (int i = 0; i < np; i++) {
		this->players[i].init(this->window, "textures/", 14, Coord(this->player_coord[i], 100+(rand() % 400)));
	}


	this->bullets = new BulletList(window);								//initiate bullet list

	for (int i = 0; i < 10; i++) {										//initiate key array, it keeps pressed keys
		keyboard_input_array[i] = 0;									//for simultaneous inputs from users 
	}																	//updated eith keypressed and keyreleased events


	this->update();						//update runs until end of the program
};




void Game::drawBackground(){												//draw background and score texts
	this->window->draw(this->bgSprite);

	string str1 = "Player 1: " + to_string(this->players[0].getScore());
	string str2 = "Player 2: " + to_string(this->players[1].getScore());
	text1.setString(str1 + "\n" + str2);
	this->window->draw(text1);

};

void Game::update(){

	while (this->window->isOpen()){
		sf::Event event;
		while (this->window->pollEvent(event)){
			if (event.type == sf::Event::Closed) {						//if event is close, close window
				this->window->close();
			}
			else if (event.type == sf::Event::KeyPressed){				//KeyPressed event
				//if else for 1st player
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
					keyboard_input_array[0] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
					keyboard_input_array[1] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
					keyboard_input_array[2] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
					keyboard_input_array[3] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
					keyboard_input_array[4] = 1;
				}

				//if else for 2nd player
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
					keyboard_input_array[5] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
					keyboard_input_array[6] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
					keyboard_input_array[7] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
					keyboard_input_array[8] = 1;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
					keyboard_input_array[9] = 1;
				}
			}
			else if (event.type == sf::Event::KeyReleased) {					//Keyreleased event
				//if else for 1st player
				if (event.key.code == sf::Keyboard::W) {
					keyboard_input_array[0] = 0;
				}
				else if (event.key.code == sf::Keyboard::D) {
					keyboard_input_array[1] = 0;
				}
				else if (event.key.code == sf::Keyboard::S) {
					keyboard_input_array[2] = 0;
				}
				else if (event.key.code == sf::Keyboard::A) {
					keyboard_input_array[3] = 0;
				}
				else if (event.key.code == sf::Keyboard::Space) {
					keyboard_input_array[4] = 0;
				}

				//if else for 2nd player
				if (event.key.code == sf::Keyboard::Up) {
					keyboard_input_array[5] = 0;
				}
				else if (event.key.code == sf::Keyboard::Right) {
					keyboard_input_array[6] = 0;
				}
				else if (event.key.code == sf::Keyboard::Down) {
					keyboard_input_array[7] = 0;
				}
				else if (event.key.code == sf::Keyboard::Left) {
					keyboard_input_array[8] = 0;
				}
				else if (event.key.code == sf::Keyboard::Enter) {
					keyboard_input_array[9] = 0;
				}
			}
		}


		for (int i = 0; i < 4; i++) {							//player 1 walk
			if (keyboard_input_array[i] == 1) {
				this->players[0].walk(this->speed, i, this->barrels, this->sandbags, this->numBarrels, this->numSandbags);
			}
		}
		if (keyboard_input_array[4] == 1) {						//player 1 shoot
			this->players[0].shoot(bullets, 0);
		}

		for (int i = 5; i < 9; i++) {							//player 2 walk
			if (keyboard_input_array[i] == 1) {
				this->players[1].walk(this->speed, i-5, this->barrels, this->sandbags, this->numBarrels, this->numSandbags);
			}
		}
		if (keyboard_input_array[9] == 1) {						//player 2 shoot
			this->players[1].shoot(bullets, 1);
		}


		this->window->clear(sf::Color::White);				//clear background
		this->drawBackground();								// draw background and texts
		

		for (int i = 0; i < this->numBarrels; i++) {		//draw barrel, sandbag and player sprites
			this->barrels[i].paint();
		}
		for (int i = 0; i < this->numSandbags; i++) {
			this->sandbags[i].paint();
		}
		for (int i = 0; i < this->numPlayers; i++) {
			if (!this->players[i].getVisible()) {							//respawn player it is shot
				this->players[i].setVisible(true);
				this->players[i].setPosition(Coord(this->player_coord[i], 100 + (rand() % 400)));
			}
			this->players[i].paint();
		}

		this->checkScore();								//checks win condition

		this->bullets->checkCollision(players, barrels, sandbags, numPlayers, numBarrels, numSandbags);
		this->bullets->update();


		this->window->display();						//display

		sf::sleep(sf::milliseconds(16));				//sleep for approximately 1/60 s
	}
};


void Game::checkScore() {
	for (int i = 0; i < this->numPlayers; i++) {
		if (this->players[i].getScore() >= 10) {									//if one them reaches score 10 
			string str = "Player " + to_string(i+1) + " Won \n Continue: Y/N";
			text2.setString(str);
			sf::FloatRect textRect = text2.getLocalBounds();
			text2.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
			text2.setPosition(sf::Vector2f(width / 2.0f, height / 2.0f));
			this->window->draw(text2);
			this->window->display();

			while (this->window->isOpen()) {								// wait for user input: yes or no
				sf::Event event;
				while (this->window->pollEvent(event)) {
					if (event.type == sf::Event::Closed) {
						this->window->close();
					}
					else if (event.type == sf::Event::KeyPressed) {
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) {
							this->reset_game();
							return;
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
							this->window->close();
						}
					}
				}
			}
		}
	}
}

void Game::reset_game() {								//resets game
	for (int i = 0; i < numBarrels; i++) {				//initiate barrels
		this->barrels[i].setVisible(true);
	}

	for (int i = 0; i < numSandbags; i++) {				//initiate sandbags
		this->sandbags[i].setVisible(true);
	}

	for (int i = 0; i < numPlayers; i++) {				//initiate players
		this->players[i].init(this->window, "textures/", 14, Coord(this->player_coord[i], 100 + (rand() % 400)));
	}
}


Game::~Game() {											//gives back allocated memory
	delete window;
	delete[] barrels;
	delete[] sandbags;
	delete[] players;
	delete bullets;
	cout << "Pointers are deleted." << endl;
}

void Player::init(sf::RenderWindow *window, string textBasePath, int numTextures, Coord pos) {
	this->window = window;
	for (int i = 0; i < numTextures; i++) {							//load textures
		this->textures[i].loadFromFile(textBasePath + "soldier" + to_string(i) + ".png");
	};
	this->sprite.setTexture(this->textures[0]);						//default texture
	this->sprite.setPosition(pos.x, pos.y);
	this->pos = pos;
	this->state = 0;
	this->s = 0;
	this->walkBool = false;
	this->score = 0;
};

void Player::setPosition(Coord pos){
	this->sprite.setPosition(pos.x, pos.y);
	this->pos = pos;
};

bool Player::checkCollision(Barrel *barrels, Sandbag *sandbags, int nb, int ns) {

	bool temp = false;

	for (int i=0; i < nb; i++) {							//check collision with barrels
		if (barrels[i].getVisible()) {
			if (this->sprite.getGlobalBounds().intersects(barrels[i].get_sprite().getGlobalBounds())) {
				temp = true;
			}
		}
	}

	for (int j = 0; j < ns; j++) {							//check collision with sandbags
		if (sandbags[j].getVisible()) {
			if (this->sprite.getGlobalBounds().intersects(sandbags[j].get_sprite().getGlobalBounds())) {
				temp = true;
			}
		}
	}
	return temp;
};

void Player::walk(float speed, int dir, Barrel *barrels, Sandbag *sandbags, int nb, int ns){

	int x = this->pos.x;
	int y = this->pos.y;

	if (this->state != 2 * dir && state<8 && !walkBool){		//if it is not facing correct direction,
		s = 0;													//turn to correct direction

		int temp = (2 * dir - this->state);						//depending on which one requires least turn, 
																//turn clockwise or counter clockwise 
		if (temp<-4 || (temp>0 && temp<4)) {		//turn clockwise
			state++;												
		}
		else {										//turn counter clockwise
			state += 7;
			state %= 8;
		}
	}
	else {															//if it is facing correct direction, walk

		if (dir == 0) {												//move on the given direction
			y += -speed;
		}
		else if (dir == 1) {
			x += speed;
		}
		else if (dir == 2) {
			y += speed;
		}
		else if (dir == 3) {
			x += -speed;
		}
		else {
			return;
		}
		this->sprite.setPosition(x, y);

		if (this->checkCollision(barrels, sandbags, nb, ns)) {		//check collision after movement
			if (dir == 0) {											//if there is collision reverse the movement
				y += speed;
			}
			else if (dir == 1) {
				x += -speed;
			}
			else if (dir == 2) {
				y += -speed;
			}
			else if (dir == 3) {
				x += speed;
			}
			cout << "Collision" << endl;
			this->sprite.setPosition(x, y);
		}
		else {											//if there is no collision keep the movement
			walkBool = false;							//walkBool shows if is character able to turns (cw or ccw)
			switch (state) {							//implementation of diagram
				case 0:
					state = (s) ? 8 : 7;
					s = (s) ? 0 : 1;
					walkBool = true;
					break;
				case 2:
					state = (s) ? 9 : 10;
					s = (s) ? 0 : 1;
					walkBool = true;
					break;
				case 3:
					state = 4;
					break;
				case 4:
					state = (s) ? 11 : 3;
					s = (s) ? 0 : 1;
					walkBool = true;
					break;
				case 6:
					state = (s) ? 12 : 13;
					s = (s) ? 0 : 1;
					walkBool = true;
					break;
				case 7:
					state = 0;
					break;
				case 8:
					state = 0;
					break;
				case 9:
					state = 2;
					break;
				case 10:
					state = 2;
					break;
				case 11:
					state = 4;
					break;
				case 12:
					state = 6;
					break;
				case 13:
					state = 6;
					break;
				default:
					break;
			}
		}
	}
	this->pos = Coord(x, y);
};


void Player::paint(){
	this->sprite.setTexture(textures[state]);
	this->window->draw(sprite);
};

void Player::shoot(BulletList *bullets, int player_num) {
	int direction = 0;
	int temp = this->state;
	if (temp == 0 || temp == 7 || temp == 8) {						//check player direction to shoot
		direction = 1;
	}
	else if (temp == 2 || temp == 0 || temp == 10) {
		direction = 2;
	}
	else if (temp == 3 || temp == 4 || temp == 11) {
		direction = 3;
	}
	else if (temp == 6 || temp == 12 || temp == 13) {
		direction = 4;
	}

	if (direction == 0) {
		return;
	}
	sf::FloatRect localBounds = this->sprite.getLocalBounds();
	int center_x = pos.x + localBounds.width/2;
	int center_y = pos.y + localBounds.height/2;
	Coord position = Coord(center_x, center_y);
	bullets->add(position, direction, player_num);
}

void Player::incrementScore(void) {
	this->score++;
}
int Player::getScore(void) {
	return this->score;
}


void Bullet::init(Coord pos, sf::RenderWindow *window, int dir, int player_number){
	this->setSpeed(10);
	this->texture.loadFromFile("textures/bullet.png");
	this->sprite.setTexture(this->texture);
	this->pos = Coord(pos.x, pos.y);
	this->next = NULL;
	this->angle = dir;
	this->sprite.rotate(90* (dir-1));
	this->window = window;
	this->isVisible = true;
	this->player = player_number;
}

void Bullet::move(void) {
	int x = 0;
	int y = 0;
	if (this->angle == 1) {
		y += -speed;
	}
	else if (this->angle == 2) {
		x += speed;
	}
	else if (this->angle == 3) {
		y += speed;
	}
	else if (this->angle == 4) {
		x -= speed;
	}
	x = this->pos.x + x;
	y = this->pos.y + y;
	this->sprite.setPosition(x, y);
	this->pos = Coord(x, y);
}

void Bullet::setSpeed(float speed) {
	this->speed = speed;
}


BulletList::BulletList(sf::RenderWindow* window){
	this->window = window;
	this->list = NULL;
}

void BulletList::add(Coord pos, int dir, int player_number) {
	if (list == NULL) {
		this->list = new Bullet;
		this->list->init(pos, window, dir, player_number);
	}
	else {
		Bullet* temp = list;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = new Bullet;
		temp->next->init(pos, window, dir, player_number);
	}
}

void BulletList::update(void) {
	if (this->list == NULL){
		return;
	}
	Bullet* temp = this->list;
	while (temp != NULL){
		if (temp->getVisible()) {					//draw if visible
			temp->move();
			temp->paint();
		}
		temp = temp->next;
	}
}


void BulletList::checkCollision(Player* players, Barrel* barrels, Sandbag* sandbags, int np, int nb, int ns) {
	if (this->list == NULL) {
		return;
	}
	Bullet* temp = this->list;
	bool temp_bool = true;
	while (temp != NULL) {
		sf::Sprite bullet_sprite = temp->get_sprite();

		temp_bool = temp->getVisible();				//a bullet can collide with only one object, if it is collided temp_bool becomes false
		if (temp_bool){
			for (int i = 0; i < nb; i++) {
				if (barrels[i].getVisible()) {
					if (bullet_sprite.getGlobalBounds().intersects(barrels[i].get_sprite().getGlobalBounds())) {
						temp_bool = false;
						temp->setVisible(false);
						barrels[i].setVisible(false);
					}
				}
			}
		}
		if (temp_bool) {
			for (int j = 0; j < ns; j++) {
				if (sandbags[j].getVisible()) {
					if (bullet_sprite.getGlobalBounds().intersects(sandbags[j].get_sprite().getGlobalBounds())) {
						temp_bool = false;
						temp->setVisible(false);
					}
				}
			}
		}
		if (temp_bool){
			for (int k = 0; k < np; k++) {
				if (bullet_sprite.getGlobalBounds().intersects(players[k].get_sprite().getGlobalBounds())){
					if (k == temp->player) {
						continue;
					}
					cout << "player collision" << endl;
					temp_bool = false;
					temp->setVisible(false);
					players[k].setVisible(false);
					players[temp->player].incrementScore();
				}
			}
		}
		temp_bool = true;
		temp = temp->next;
	}
}



int main(){

	Game game_obj(5, 800, 600, 3, 5, 2);					//start the game

	//sandbag barrel and player numbers are fixed and it should not be changed

	return 0;
};