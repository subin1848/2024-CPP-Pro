#include<SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include<ctime>
#include<cstdlib>
#include<vector>
#include<iostream>


using namespace sf;
using namespace std;

#define WIDTH 800		//����
#define HEIGHT 600		//����
#define TOTAL_HEIGHT (HEIGHT * 10)	//���� ��ü ����
#define BAR_COUNT 170	//��� bar ����
static const float GRAVITY = 0.2f;	//�߷�

// ���� �� ���� ����
const int MIN_GAP = 100;	// bar�� �ּ� ����
const int MAX_GAP = 200;

enum PlayerState { READY, JUMPING, FALLING };


// ���ȭ�� class
class Background
{
private:
	vector<Texture> textures;
	vector<Sprite> backgroundSprites;
	int currentBackgroundIndex;
	float backgroundHeight;
	float fadeAlpha;
	bool isFading;
	float totalHeight;
	const float CHANGE_HEIGHT = 1000.0f;

	void addNewBackground(float yPos) {
		int index = backgroundSprites.size() % textures.size();
		Sprite sprite(textures[index]);

		float scaleX = static_cast<float>(WIDTH) / sprite.getLocalBounds().width;
		float scaleY = static_cast<float>(HEIGHT) / sprite.getLocalBounds().height;
		sprite.setScale(scaleX, scaleY);

		sprite.setPosition(0, yPos);
		backgroundSprites.push_back(sprite);
		totalHeight += HEIGHT;
	}

public:
	Background()
	{
		std::vector<std::string> textureFiles = {
			"assets/images/street.png",
			"assets/images/home.png",
			"assets/images/playing_room.png",
			"assets/images/hospital.png"
		};

		// �ؽ�ó�� �ε��ϰ� ���Ϳ� ����
		for (const auto& file : textureFiles) {
			Texture texture;
			if (!texture.loadFromFile(file)) {
				throw std::runtime_error("Failed to load texture: " + file);
			}
			textures.push_back(texture);
		}

		addNewBackground(0);
		totalHeight = HEIGHT;

		// ��������Ʈ�� �ؽ�ó�� �ʱ�ȭ
		for (size_t i = 0; i < textures.size(); ++i) {
			Sprite sprite(textures[i]);

			// ȭ�� ũ�⿡ �°� ������ ����
			float scaleX = static_cast<float>(WIDTH) / sprite.getLocalBounds().width;
			float scaleY = static_cast<float>(HEIGHT) / sprite.getLocalBounds().height;
			sprite.setScale(scaleX, scaleY);

			// ������ �������� �̾����
			sprite.setPosition(0, i * HEIGHT);
			backgroundSprites.push_back(sprite);
		}

		currentBackgroundIndex = 0;
		backgroundHeight = HEIGHT * textures.size();

	}

	void update(float playerHeight)
	{
		// ���ο� ��� �߰� �ʿ� ���� Ȯ��
		while (playerHeight > totalHeight - HEIGHT) {
			addNewBackground(totalHeight);
		}

		// ��� ��ġ ������Ʈ
		for (auto& sprite : backgroundSprites) {
			sprite.setPosition(0, totalHeight - playerHeight - sprite.getGlobalBounds().height);
		}

	}

	void draw(RenderWindow& window)
	{
		for (const auto& sprite : backgroundSprites) {
			window.draw(sprite);
		}
	}
	void startFadeToNextBackground()
	{
		isFading = true;
		fadeAlpha = 255;
	}
	//	float getHeight() const
	//	{
	//		return sprite.getGlobalBounds().height;
	//	}
};


// �÷��̾� Ŭ����
class Player
{
private:
	int x, y;
	int imgWidth, imgHeight;
	float dy;		// ���� �̵� �ӵ�
	Sprite* imgJump;
	Sprite* imgReady;

	PlayerState currentState;

	Texture t1, t2;
	//sound
	sf::SoundBuffer jumpSoundBuffer;
	sf::Sound jumpSound;

	const float GRAVITY = 0.5f;      // �߷�
	const float JUMP_FORCE = -15.0f; // ���� �ʱ� �ӵ�

private:
	const Sprite& GetImg()		// ���� ���� �̹��� ��ȯ
	{
		return (currentState == JUMPING) ? *imgJump : *imgReady;
	}

public:
	Player() : dy(0), currentState(READY)	// �ʱ�ȭ �۾� ����
	{
		x = WIDTH / 2;
		y = HEIGHT / 2;

		t1.loadFromFile("assets/images/dog_right.png");
		t2.loadFromFile("assets/images/dog_left.png");

		imgJump = new Sprite(t1);
		imgReady = new Sprite(t2);


		imgJump->setScale(0.3f, 0.3f);
		imgReady->setScale(0.3f, 0.3f);

		imgWidth = static_cast<int>(imgReady->getTexture()->getSize().x * 0.3);
		imgHeight = static_cast<int>(imgReady->getTexture()->getSize().y * 0.3);


		if (!jumpSoundBuffer.loadFromFile("assets/sounds/Jumping_Sound.mp3")) {
			// ���� �ε� ���� ó��
			cout << "Failed to load jump sound" << endl;
		}
		jumpSound.setBuffer(jumpSoundBuffer);
	}
	~Player()
	{
		delete(imgJump);
		delete(imgReady);
	}

	void SetPosition()
	{
		imgReady->setPosition(x, y);
		imgJump->setPosition(x, y);
	}
	float GetCurrentHeight() const {
		return TOTAL_HEIGHT - y;
	}
	void Move()
	{
		if (Keyboard::isKeyPressed(Keyboard::Right)) //�������̵�
		{
			x += 5;
			imgReady = new Sprite(t1);  // dog_right
			imgReady->setScale(0.3f, 0.3f);
		}
		if (Keyboard::isKeyPressed(Keyboard::Left)) //�����̵�
		{
			x -= 5;
			imgReady = new Sprite(t2);  // dog_right
			imgReady->setScale(0.3f, 0.3f);
		}
		if (x < 0)	//���� �� ���� ���ϰ�
		{
			x = 0;
		}
		if (x > WIDTH - imgWidth)	//������ �� ���� ���ϰ�
		{
			x = WIDTH - imgWidth;
		}

		dy += GRAVITY;
		y += static_cast<int>(dy);


		if (dy > 0)
		{
			currentState = FALLING;
		}
		// ���� ���� �߰�
		if (dy < -15) // �ִ� ��� �ӵ� ����
		{
			dy = -15;
		}
		else if (dy > 15) // �ִ� �ϰ� �ӵ� ����
		{
			dy = 15;
		}
	}
	void Draw(RenderWindow& window)		// ȭ�鿡 �̹��� �׸���
	{
		imgReady->setPosition(x, y);

		if (currentState == JUMPING) {
			window.draw(*imgJump);
		}
		else {
			window.draw(*imgReady);
		}
	}

	// ���� ����
	void Jump()
	{
		if (currentState == READY || currentState == FALLING)
		{
			currentState = JUMPING;
			dy = JUMP_FORCE;  // �ʱ� ��� �ӵ�
			float heightInMeters = GetCurrentHeight() / 10.0f;
			float baseJumpVelocity = -10.0f;

			// �ӵ� �ν�Ʈ 20m ���� ���� �ӵ� ����
			if (heightInMeters > 20.0f)
			{
				float speedBoostFactor = 1.0f + (heightInMeters - 20.0f) * 0.1f;
				// ������ ���� ������ ���� �ӵ� �ν�Ʈ ���Ѽ� ����
				speedBoostFactor = min(speedBoostFactor, 2.0f);
				dy = baseJumpVelocity * speedBoostFactor;
			}
			else {
				dy = baseJumpVelocity;
			}

			jumpSound.play();
		}
	}
	// ��� ��ȯ �� ���ϼӵ� ����
	void ResetJumpState()
	{
		// ���� ���� �ʱ�ȭ
		currentState = FALLING;
		dy = 0.5f; // �ణ�� ���� �ӵ� �ο�
	}
	bool CheckCollision(int barX, int barY, int barWidth, int barHeight)
	{
		return (x < barX + imgWidth * 0.15f &&
			x + imgWidth > barX &&
			y + imgHeight >= barY &&
			y + imgHeight <= barY + imgHeight * 0.15f &&
			dy > 0);
	}
	bool IsOutOfScreen(const View& view) const
	{
		return y > view.getCenter().y + HEIGHT / 2 + imgHeight; // ȭ�� �Ʒ��� ������ ���
	}

	float GetDy() const
	{
		return dy;
	}
	int GetY() const
	{
		return y;
	}
	int GetX() const
	{
		return x;
	}
	int GetWidth() const
	{
		return imgWidth;
	}
	int GetHeight() const
	{
		return imgHeight;
	}
	void SetY(int _y)
	{
		y = _y;
	}

};

//const float Player::MAX_FALL_TIME = 2.0f; // 2�� ���� ������ �� ����

// ���� bar Ŭ����
struct Pos		// bar�� ��ġ ����ü
{
	int x;
	int y;
};

class Bar
{
public:
	vector<Pos> vBar;	// bar�� ��ġ ���� ����
	Sprite* imgBar;		// bar �̹��� ��Ÿ�� (sprite ��ü)
	Texture t;			// bar �̹��� �ε�
	int imgWidth;
	int imgHeight;

	Bar()
	{
		srand(static_cast<unsigned int>(time(nullptr)));

		t.loadFromFile("assets/images/bush.png");
		t.loadFromFile("assets/images/dog_treats.png");
		t.loadFromFile("assets/images/dog_house.png");

		imgBar = new Sprite(t);
		imgBar->setScale(0.2f, 0.08f);

		imgWidth = static_cast<int>(imgBar->getTexture()->getSize().x);
		imgHeight = static_cast<int>(imgBar->getTexture()->getSize().y);

		// ȭ���� �յ��� �������� ������
		int sectionWidth = WIDTH / BAR_COUNT;

		// �ʱ� �� ��ġ ����
		int currentY = HEIGHT;
		for (int i = 0; i < BAR_COUNT; ++i)
		{
			Pos p;
			// �� ���� ������ ������ X ��ǥ ����
			p.x = (WIDTH / 2) - (imgWidth / 2) + (rand() % (sectionWidth - imgWidth));

			// ���� Y ��ǥ ������ ����
			p.y = currentY + rand() % (MAX_GAP - MIN_GAP + 1) - MIN_GAP;			// �ּ�/�ִ� ���� Y ��ǥ
			currentY = p.y;

			vBar.push_back(p);
		}
	}
	~Bar()
	{
		delete(imgBar);
	}

	void Draw(RenderWindow& window)
	{
		for (const auto& bar : vBar)
		{
			imgBar->setPosition(bar.x, bar.y);
			window.draw(*imgBar);
		}
	}

	// cloud�� player�� �浹 ����
	bool CheckCollision(Player* pPlayer)
	{
		for (const auto& bar : vBar)
		{
			if (pPlayer->CheckCollision(bar.x, bar.y, imgWidth, imgHeight))
			{
				pPlayer->SetY(bar.y - pPlayer->GetHeight());
				pPlayer->Jump();
				return true; // �浹 �߻�
			}
		}
		return false;
	}

	// �÷��� ��ġ ����
	void MoveAndReset(Player* pPlayer)
	{
		for (auto& bar : vBar)
		{
			if (bar.y > pPlayer->GetY() + HEIGHT) // ȭ�� ������ ��� ���
			{
				bar.y -= TOTAL_HEIGHT;           // �÷��� ����
				bar.x = (WIDTH / 2) - (imgWidth / 2) + (rand() % (WIDTH - imgWidth)); // �߾� �������� ���ο� X ��ǥ ���
			}
		}
	}

};

int main(void)
{
	RenderWindow window(VideoMode(WIDTH, HEIGHT), "C++ Jump Game");
	window.setFramerateLimit(60);

	Player* pPlayer = new Player();
	Bar* pBar = new Bar();
	Background background;

	float lastBackgroundChangeY = 0;
	float lastConsolePrintY = 0;
	float lastBackgroundChangeHeight = 0;
	float currentHeight;


	// ���� ���� ���� ���� �߰�
	sf::Clock fallClock;
	sf::Clock deltaClock;
	sf::Font font;

	sf::Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setString("Game Over!");
	gameOverText.setCharacterSize(50);
	gameOverText.setFillColor(Color::Red);
	gameOverText.setPosition(WIDTH / 2 - 100, HEIGHT / 2);

	sf::Text heightText;
	heightText.setFont(font);
	heightText.setCharacterSize(20);
	heightText.setFillColor(sf::Color::White);
	heightText.setPosition(10, 10);

	float maxHeight = 0.0f;

	bool gameOver = false;

	View view(FloatRect(0, 0, WIDTH, HEIGHT));

	while (window.isOpen())
	{
		float deltaTime = deltaClock.restart().asSeconds();
		Event e;

		while (window.pollEvent(e))
		{
			if (e.type == Event::Closed)
				window.close();
		}

		if (!gameOver)
		{
			// ���� ����
			pPlayer->Move();
			pBar->CheckCollision(pPlayer);
			pBar->MoveAndReset(pPlayer);
			pPlayer->SetPosition();

			float currentHeight = pPlayer->GetCurrentHeight();
			background.update(currentHeight);

			// �ֿܼ� ���� ���� ��� (0.5�ʸ���)
			if (currentHeight - lastConsolePrintY >= 50) // 50cm���� ���
			{
				std::cout << "Height: " << currentHeight / 10.0f << " meters" << std::endl;
				lastConsolePrintY = currentHeight;
			}

			//��� ��ȯ üũ
			if (currentHeight - lastBackgroundChangeHeight >= 1000)
			{
				background.startFadeToNextBackground();
				lastBackgroundChangeHeight = currentHeight;
				std::cout << "Background changed at " << currentHeight / 10.0f << " meters" << std::endl;
				// ��� ��ȯ �� ��� ���� ����
				pPlayer->ResetJumpState();
			}
			background.update(currentHeight);

			// ȭ�� ���󰡱�
			view.setCenter(WIDTH / 2, pPlayer->GetY() + HEIGHT / 3);

			// ���� ���� ���� Ȯ��
			if (pPlayer->IsOutOfScreen(view))
			{
				gameOver = true;
				fallClock.restart();
			}

			currentHeight = pPlayer->GetCurrentHeight();
			if (currentHeight > maxHeight)
			{
				maxHeight = currentHeight;
			}

			// ȭ�� �׸���
			window.clear();
			window.setView(view);

			background.draw(window);
			pBar->Draw(window);
			pPlayer->Draw(window);

			heightText.setString("Height: " + std::to_string(static_cast<int>(maxHeight / 10)) + "cm");
			window.draw(heightText);

			window.display();

		}
		else
		{
			// ���� ���� ���¿��� 2�� �� â �ݱ�
			window.clear();
			window.setView(window.getDefaultView());
			window.draw(gameOverText);
			if (fallClock.getElapsedTime().asSeconds() > 2.0f)
			{
				window.close();
			}

			window.display();
		}
	}

	delete(pBar);
	delete(pPlayer);
	return 0;
};



