#include <SFML/Graphics.hpp>
#include "map.h"
#include "view.h"
#include <sstream>

using namespace sf;

class Entity {
public:
	float dx, dy, x, y, speed, moveTimer;
	int w, h, health;
	bool life, isMove, onGround;
	Texture texture;
	Sprite sprite;
	String name;
	Entity(Image &image, float X, float Y, int W,int H, String Name){
		x = X; y = Y; w = W; h = H; name = Name; moveTimer = 0;
		speed = 0; health = 100; dx = 0; dy = 0;
		life = true; onGround = false; isMove = false;
		texture.loadFromImage(image);
		sprite.setTexture(texture);
		sprite.setOrigin(w / 2, h / 2);
	}
};




class Player :public Entity {
public:
	int score;
	enum { left, right, up, down, jump, stay } state;

	Player(Image &image, float X, float Y, int W, int H, String Name) :Entity(image, X, Y, W, H, Name) {
		score = 0;				
		state = stay;
		if (name == "Player1") {
			sprite.setTextureRect(IntRect(0, 0, w, h));
		}
	}

	void control() 
	{
		if (Keyboard::isKeyPressed) {
			if (Keyboard::isKeyPressed(Keyboard::Left))
			{
				state = left;
				speed = 0.1;
			}
			if (Keyboard::isKeyPressed(Keyboard::Right))
			{
				state = right;
				speed = 0.1;
			}
			if ((Keyboard::isKeyPressed(Keyboard::Up)) && (onGround))
			{
				state = jump;
				dy = -0.8; onGround = false;
			}
			if (Keyboard::isKeyPressed(Keyboard::Down))
			{
				state = down;
			}
		}
	}

	void update(float time)
	{
		control();
		switch (state)
		{
		case right: dx = speed; break;
		case left: dx = -speed; break;
		case up: break;
		case down: dx = 0;  break;
		case stay: break;
		}
		x += dx*time;
		checkCollisioWithMap(dx, 0);
		y += dy*time;
		checkCollisioWithMap(0, dy);
		sprite.setPosition(x+w/2, y+h/2);
		if (health <= 0) { life = false; }
		if (!isMove) { speed = 0; }
		if (life) { setplayercoordinateforview(x, y); }
		dy = dy + 0.0015*time;		
	}

	void checkCollisioWithMap(float Dx, float Dy)//ф ция проверки столкновений с картой
	{
		for (int i = y / 32; i < (y + h) / 32; i++)//проходимся по элементам карты
			for (int j = x / 32; j<(x + w) / 32; j++)
			{
				if (TileMap[i][j] == '0')//если элемент наш тайлик земли? то
				{
					if (Dy>0) { y = i * 32 - h;  dy = 0; onGround = true; }//по Y вниз=>идем в пол(стоим на месте) или падаем. В этот момент надо вытолкнуть персонажа и поставить его на землю, при этом говорим что мы на земле тем самым снова можем прыгать
					if (Dy<0) { y = i * 32 + 32;  dy = 0; }//столкновение с верхними краями карты(может и не пригодиться)
					if (Dx>0) { x = j * 32 - w; }//с правым краем карты
					if (Dx<0) { x = j * 32 + 32; }// с левым краем карты
				}
				else { onGround = false; }
			}
	}

	float GetPlayerCoordinateX() { return x; }
	float GetPlayerCoordinateY() { return y; }
};

class Enemy :public Entity {
public:
	Enemy(Image &image, float X, float Y, int W, int H, String Name) :Entity(image, X, Y, W, H, Name) {
		if (name == "EasyEnemy") {
			sprite.setTextureRect(IntRect(0, 0, w, h));
			dx = 0.1;
		}
	}

	void checkCollisionWithMap(float Dx, float Dy)//ф ция проверки столкновений с картой
	{
		for (int i = y / 32; i < (y + h) / 32; i++)//проходимся по элементам карты
			for (int j = x / 32; j < (x + w) / 32; j++)
			{
				if (TileMap[i][j] == '0')//если элемент наш тайлик земли? то
				{
					if (Dy > 0) { y = i * 32 - h; }//по Y вниз=>идем в пол(стоим на месте) или падаем. В этот момент надо вытолкнуть персонажа и поставить его на землю, при этом говорим что мы на земле тем самым снова можем прыгать
					if (Dy < 0) { y = i * 32 + 32; }//столкновение с верхними краями карты(может и не пригодиться)
					if (Dx > 0) { x = j * 32 - w;  dx = -0.1; sprite.scale(-1, 1); }//с правым краем карты
					if (Dx < 0) { x = j * 32 + 32; dx = 0.1; sprite.scale(-1, 1); }// с левым краем карты
				}
			}
	}

	void update(float time)
	{
		if (name == "EasyEnemy") {
			moveTimer += time; if (moveTimer > 3000) { dx *= -1; moveTimer = 0; }
			checkCollisionWithMap(dx, 0);
			x += dx*time;
			sprite.setPosition(x + w / 2, y + h / 2);
			if (health <= 0) { life = false; }
		}
	}
};

int main()
{
	RenderWindow window(sf::VideoMode(640, 480), "KEK");
	view.reset(sf::FloatRect(0, 0, 640, 480));

	Image heroImage;
	heroImage.loadFromFile("images/Mainhero.png");

 	Image map_image;
	map_image.loadFromFile("images/map.png");
	Texture map;
	map.loadFromImage(map_image);
	Sprite s_map;
	s_map.setTexture(map);
	
	Image easyEnemyImage;
	easyEnemyImage.loadFromFile("images/shamaich.png");
	easyEnemyImage.createMaskFromColor(Color(255, 0, 0));

	Player p(heroImage, 250, 250, 95, 107, "Player1");
	Enemy easyEnemy(easyEnemyImage, 850, 671, 200, 97, "EasyEnemy");

	Clock clock;
	Clock gameTimeClock;
	int gameTime = 0;
	while (window.isOpen())
	{
		float time = clock.getElapsedTime().asMicroseconds();
		if (p.life) gameTime = gameTimeClock.getElapsedTime().asSeconds();
		clock.restart();
		time = time / 800;
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();
		}
		
		p.update(time);
		easyEnemy.update(time);
		window.setView(view);
		window.clear();
		for (int i = 0; i < HEIGHT_MAP; i++)
			for (int j = 0; j < WIDTH_MAP; j++)
			{
				if (TileMap[i][j] == ' ')  s_map.setTextureRect(IntRect(0, 0, 32, 32));
				if (TileMap[i][j] == 's')  s_map.setTextureRect(IntRect(32, 0, 32, 32));
				if ((TileMap[i][j] == '0')) s_map.setTextureRect(IntRect(64, 0, 32, 32));
				if ((TileMap[i][j] == 'f')) s_map.setTextureRect(IntRect(96, 0, 32, 32));//добавили цветок
				if ((TileMap[i][j] == 'h')) s_map.setTextureRect(IntRect(128, 0, 32, 32));//и сердечко
				s_map.setPosition(j * 32, i * 32);
				window.draw(s_map);
			}
		window.draw(easyEnemy.sprite);
		window.draw(p.sprite);
		window.display();
	}
	return 0;
}

