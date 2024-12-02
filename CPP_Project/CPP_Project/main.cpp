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

// �÷��̾� ���� ������ ����
enum PlayerState { READY, JUMPING, FALLING };

// ���ȭ�� class
class Background
{
private:
	Texture backgroundTexture;	// ��� �ؽ�ó
	Sprite backgroundSprite;	// ��� ��������Ʈ
	float currentHeight;		// ���� ����
	float fadeAlpha;			// ���̵� ȿ�� ���İ�
	bool isFading;				// ���̵� ���� �÷���
	float totalHeight;			// ���̵� ���� �÷���
	bool barStateChanged;		//bar ���� ����

public:
	// ������: ��� �̹��� ���� ��θ� �޾� �ʱ�ȭ
	Background(const std::string& filePath)
	{
		// �� ��� �̹��� �ε�
		if (!backgroundTexture.loadFromFile(filePath)) {
			throw std::runtime_error("Failed to load texture: " + filePath);
		}

		backgroundSprite.setTexture(backgroundTexture);

		// �ʱ� �� ����
		currentHeight = 0;
		totalHeight = backgroundSprite.getLocalBounds().height * backgroundSprite.getScale().y;
		barStateChanged = false;
	}

	// ��� ������Ʈ �Լ�
	void update(float currentHeight, float deltaTime) 
	{
		// ��ũ�� �ӵ� ���� (��: 100�ȼ�/��)
		float scrollSpeed = 100.0f;
		currentHeight += scrollSpeed * deltaTime;

		// ����� ������ �ٽ� ����. �̰� ������ ��� ���� �̻�����
		if (currentHeight >= totalHeight) {
			currentHeight = 0;
			barStateChanged = false; // ��� ���� �� bar ���� �ʱ�ȭ
		}

		// ��� ��ġ ������Ʈ
		backgroundSprite.setPosition(0, -fmod(currentHeight, totalHeight));

		// y ��ǥ�� 150 �̻��� �� bar ���� ����
		if (currentHeight >= 150 && !barStateChanged) {
			barStateChanged = true;
			changeBarState(); // bar ���¸� �����ϴ� �Լ� ȣ��
		}
	}
	// ��� �׸���
	void draw(RenderWindow& window)
	{
		window.draw(backgroundSprite);
	}
	// Bar ���� ���� �Լ�
	void changeBarState()
	{
		std::cout << "Bar ���°� ����Ǿ����ϴ�!" << std::endl;
	}

	// ���� ������� ���̵� ȿ�� 
//	void startFadeToNextBackground()
//	{
//		isFading = true;
//		fadeAlpha = 255;
//	}

};


// �÷��̾� Ŭ����
class Player
{
private:
	int x, y;				// �÷��̾� ��ġ
	int imgWidth, imgHeight;// �÷��̾� �̹��� ũ��
	float dy;				// ���� �̵� �ӵ�
	Sprite* imgJump;		// ���� �� �̹���
	Sprite* imgReady;		// ��� �� �̹���

	PlayerState currentState;	// ���� ����
		
	Texture t1, t2;

	// ���� sound
	sf::SoundBuffer jumpSoundBuffer;
	sf::Sound jumpSound;

	const float GRAVITY = 0.5f;      // �߷�
	const float JUMP_FORCE = -15.0f; // ���� �ʱ� �ӵ�

private:
	// ���¿� ���� �̹��� ��ȯ
	const Sprite& GetImg()		// ���� ���� �̹��� ��ȯ
	{
		return (currentState == JUMPING) ? *imgJump : *imgReady;
	}

public:
	// ������ : �⺻ �� �ʱ�ȭ
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

	// �÷��̾� ��ġ ����
	void SetPosition()
	{
		imgReady->setPosition(x, y);
		imgJump->setPosition(x, y);
	}
	// ���� ���� ��ȯ
	float GetCurrentHeight() const {
		return TOTAL_HEIGHT - y;
	}
	// �÷��̾� �̵� ó��
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

		// ȭ�� ���
		if (x < 0)	//���� �� ���� ���ϰ�
		{
			x = 0;
		}
		if (x > WIDTH - imgWidth)	//������ �� ���� ���ϰ�
		{
			x = WIDTH - imgWidth;
		}

		// �߷� �ۿ�
		dy += GRAVITY;
		y += static_cast<int>(dy);

		// ���� ������Ʈ
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

	// ȭ�鿡 �̹��� �׸���
	void Draw(RenderWindow& window)
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
		// ���� ���°� READY(�غ�) �Ǵ� FALLING(���� ��)�� ���� ���� ����
		if (currentState == READY || currentState == FALLING)
		{
			currentState = JUMPING;	// ���� ���·� ����
			dy = JUMP_FORCE;		// �ʱ� ��� �ӵ�
			float heightInMeters = GetCurrentHeight() / 10.0f;		// ���� ���̸� ���� ������ ��ȯ
			float baseJumpVelocity = -10.0f;	// �⺻ ���� �ӵ�

			// �ӵ� �ν�Ʈ 20m ���� ���� �ӵ� ����
			if (heightInMeters > 20.0f)
			{
				// ���̿� ���� �ӵ� ����
				float speedBoostFactor = 1.0f + (heightInMeters - 20.0f) * 0.1f;
				// ������ ���� ������ ���� �ӵ� �ν�Ʈ ���Ѽ� ����
				speedBoostFactor = min(speedBoostFactor, 2.0f);
				dy = baseJumpVelocity * speedBoostFactor;
			}
			else 
			{
				dy = baseJumpVelocity;		// �⺻ ���� �ӵ� ���
			}

			jumpSound.play();		
		}
	}
	// ���� ó�� �Լ�
	void Land() {
		currentState = READY;	// READY ���·� ����
		dy = 0;					// ���� �ӵ� �ʱ�ȭ
	}
	// ��� ��ȯ �� ���ϼӵ� ����
	void ResetJumpState()
	{
		currentState = FALLING;	// FALLING ���·� ����
		dy = 0.5f;				// �ణ�� ���� �ӵ� �ο�
	}

	// �÷��̾�� ��ֹ�(bar) �浹 üũ
	bool CheckCollision(int barX, int barY, int barWidth, int barHeight)
	{
		return (x < barX + imgWidth * 0.15f &&	// x�� �浹
			x + imgWidth > barX &&				
			y + imgHeight >= barY &&			// Y�� �浹
			y + imgHeight <= barY + imgHeight * 0.15f &&
			dy > 0);							// �÷��̾ �Ʒ��� �̵� ��
	}
	// ȭ�� ������ ������� Ȯ��
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

// ���� bar Ŭ����
struct Pos		// bar�� ��ġ ����ü
{
	int x;
	int y;
};

class Bar
{
//private:
//	RectangleShape barShape; // ���൵�� ��Ÿ���� ��
//	float maxWidth;          // ���� �ִ� �ʺ�
//	float currentProgress;   // ���� ���൵ (0 ~ 100)
//	Color normalColor;
//	Color activeColor;

public:
	vector<Pos> vBar;	// bar�� ��ġ ���� ����
	Sprite* imgBar;		// bar �̹��� ��Ÿ�� (sprite ��ü)
	Texture t;			// bar �̹��� �ε�
	Texture t2, t3, t4;	// �ٸ� bar �ؽ�ó��
	int imgWidth;
	int imgHeight;

	Bar()
	{
		srand(static_cast<unsigned int>(time(nullptr)));

		t.loadFromFile("assets/images/bush.png");
		t2.loadFromFile("assets/images/dog_treats.png");
		t3.loadFromFile("assets/images/dog_house.png");
		t4.loadFromFile("assets/images/cloud.png");

		imgBar = new Sprite(t);
		imgBar->setScale(0.1f, 0.07f);

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

			vBar.push_back(p);	// ��ġ ������ ���Ϳ� ����
		}
	}
	~Bar()
	{
		delete(imgBar);
	}

	// bar �׸���
	void Draw(RenderWindow& window)
	{
		for (const auto& bar : vBar)
		{
			imgBar->setPosition(bar.x, bar.y);
			window.draw(*imgBar);
		}
	}

	// bar�� y ��ǥ�� Ư�� ���� �ʰ����� �� �ؽ�ó ����
	void UpdateTextures()
	{
		// �ؽ�ó ���� ������ �����ϴ� ����
		static int textureIndex = 0;

		for (auto& bar : vBar)
		{
			if (bar.y > 145) // y ��ǥ�� 145�� �ʰ��ϸ� �ؽ�ó ����
			{
				switch (textureIndex)
				{
					case 0: imgBar->setTexture(t); break;
					case 1: imgBar->setTexture(t2); break;
					case 2: imgBar->setTexture(t3); break;
					case 3: imgBar->setTexture(t4); break;
					default: imgBar->setTexture(t); break;
				}
				// �ؽ�ó �ε����� ���������� ������Ű�� 4�� �Ǹ� �ٽ� 0���� ����
				textureIndex = (textureIndex + 1) % 4;

				// �ش� bar�� y ��ǥ�� �ʱ�ȭ �Ǵ� ���ο� ��ġ�� �̵�
				bar.y = rand() % 100; // ���Ƿ� �缳��
			}
		}
	}

	// cloud�� player�� �浹 ����
	bool CheckCollision(Player* pPlayer)
	{
		for (const auto& bar : vBar) 
		{
			if (pPlayer->CheckCollision(bar.x, bar.y, imgWidth, imgHeight)) 
			{
				pPlayer->SetY(bar.y - pPlayer->GetHeight());	// �浹 �� �÷��̾� ��ġ ����
				pPlayer->Land();
				return true;
			}
		}
		return false;
	}

	// bar ��ġ ����
	void MoveAndReset(Player* pPlayer)
	{
		for (auto& bar : vBar)
		{
			if (bar.y > pPlayer->GetY() + HEIGHT) // ȭ�� ������ ��� ���
			{
				bar.y -= TOTAL_HEIGHT;           // bar ��ġ�� ���� �̵�
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

	Player player;
	Bar barSystem;

	Background background("assets/images/backgrounds.png");

	// ���� ���� ���� �ʱ�ȭ
	float lastBackgroundChangeY = 0;
	float lastConsolePrintY = 0;
	float lastBackgroundChangeHeight = 0;
	float currentHeight;


	// ���� ���� ���� ���� �߰�
	sf::Clock fallClock;	// ���� �ð�
	sf::Clock deltaClock;	// �ð� ����
	sf::Font font;

	// game over
	sf::Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setString("Game Over!");
	gameOverText.setCharacterSize(50);
	gameOverText.setFillColor(Color::Red);
	gameOverText.setPosition(WIDTH / 2 - 100, HEIGHT / 2);

	// ���� ǥ�� �ؽ�Ʈ �ʱ�ȭ
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
			if (pBar->CheckCollision(pPlayer)) {
				pPlayer->Jump();
			};
			barSystem.MoveAndReset(&player); // bar ��ġ ����
			barSystem.UpdateTextures();     // �ؽ�ó ������Ʈ

			// ȭ�� �ʱ�ȭ
			window.clear();
			barSystem.Draw(window);         // bar �׸���
			player.Draw(window);            // �÷��̾� �׸���
			window.display();
			pPlayer->SetPosition();			// �÷��̾� ��ġ ����

			float currentHeight = pPlayer->GetCurrentHeight();		// ���� ���� ��������
			background.update(currentHeight, deltaTime);			// ��� ������Ʈ

			// �ֿܼ� ���� ���� ���
			if (currentHeight - lastConsolePrintY >= 1000) // 10m���� ���
			{
				std::cout << "Height: " << currentHeight / 10.0f << " meters" << std::endl;
				lastConsolePrintY = currentHeight;
			}

			// ȭ�� ���󰡱�
			view.setCenter(WIDTH / 2, pPlayer->GetY() + HEIGHT / 3);

			// ���� ���� ���� Ȯ��
			if (pPlayer->IsOutOfScreen(view))
			{
				gameOver = true;
				fallClock.restart();
			}

			currentHeight = pPlayer->GetCurrentHeight();
			if (currentHeight > maxHeight)	// �ִ� ���� ��� ����
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