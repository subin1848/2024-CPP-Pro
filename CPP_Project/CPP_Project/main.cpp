#include<SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include<ctime>
#include<cstdlib>
#include<vector>
#include<iostream>


using namespace sf;
using namespace std;

#define WIDTH 800		//가로
#define HEIGHT 600		//세로
#define TOTAL_HEIGHT (HEIGHT * 10)	//게임 전체 높이
#define BAR_COUNT 170	//밟는 bar 개수
static const float GRAVITY = 0.2f;	//중력

// 랜덤 바 간격 설정
const int MIN_GAP = 100;	// bar의 최소 간격
const int MAX_GAP = 200;

enum PlayerState { READY, JUMPING, FALLING };


// 배경화면 class
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

		// 텍스처를 로드하고 벡터에 저장
		for (const auto& file : textureFiles) {
			Texture texture;
			if (!texture.loadFromFile(file)) {
				throw std::runtime_error("Failed to load texture: " + file);
			}
			textures.push_back(texture);
		}

		addNewBackground(0);
		totalHeight = HEIGHT;

		// 스프라이트를 텍스처로 초기화
		for (size_t i = 0; i < textures.size(); ++i) {
			Sprite sprite(textures[i]);

			// 화면 크기에 맞게 스케일 조정
			float scaleX = static_cast<float>(WIDTH) / sprite.getLocalBounds().width;
			float scaleY = static_cast<float>(HEIGHT) / sprite.getLocalBounds().height;
			sprite.setScale(scaleX, scaleY);

			// 배경들을 수직으로 이어붙임
			sprite.setPosition(0, i * HEIGHT);
			backgroundSprites.push_back(sprite);
		}

		currentBackgroundIndex = 0;
		backgroundHeight = HEIGHT * textures.size();

	}

	void update(float playerHeight)
	{
		// 새로운 배경 추가 필요 여부 확인
		while (playerHeight > totalHeight - HEIGHT) {
			addNewBackground(totalHeight);
		}

		// 배경 위치 업데이트
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


// 플레이어 클래스
class Player
{
private:
	int x, y;
	int imgWidth, imgHeight;
	float dy;		// 수직 이동 속도
	Sprite* imgJump;
	Sprite* imgReady;

	PlayerState currentState;

	Texture t1, t2;
	//sound
	sf::SoundBuffer jumpSoundBuffer;
	sf::Sound jumpSound;

	const float GRAVITY = 0.5f;      // 중력
	const float JUMP_FORCE = -15.0f; // 점프 초기 속도

private:
	const Sprite& GetImg()		// 점프 중인 이미지 반환
	{
		return (currentState == JUMPING) ? *imgJump : *imgReady;
	}

public:
	Player() : dy(0), currentState(READY)	// 초기화 작업 설정
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
			// 사운드 로딩 실패 처리
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
		if (Keyboard::isKeyPressed(Keyboard::Right)) //오른쪽이동
		{
			x += 5;
			imgReady = new Sprite(t1);  // dog_right
			imgReady->setScale(0.3f, 0.3f);
		}
		if (Keyboard::isKeyPressed(Keyboard::Left)) //왼쪽이동
		{
			x -= 5;
			imgReady = new Sprite(t2);  // dog_right
			imgReady->setScale(0.3f, 0.3f);
		}
		if (x < 0)	//왼쪽 벽 뚫지 못하게
		{
			x = 0;
		}
		if (x > WIDTH - imgWidth)	//오른쪽 벽 뚫지 못하게
		{
			x = WIDTH - imgWidth;
		}

		dy += GRAVITY;
		y += static_cast<int>(dy);


		if (dy > 0)
		{
			currentState = FALLING;
		}
		// 점프 제한 추가
		if (dy < -15) // 최대 상승 속도 제한
		{
			dy = -15;
		}
		else if (dy > 15) // 최대 하강 속도 제한
		{
			dy = 15;
		}
	}
	void Draw(RenderWindow& window)		// 화면에 이미지 그리기
	{
		imgReady->setPosition(x, y);

		if (currentState == JUMPING) {
			window.draw(*imgJump);
		}
		else {
			window.draw(*imgReady);
		}
	}

	// 점프 시작
	void Jump()
	{
		if (currentState == READY || currentState == FALLING)
		{
			currentState = JUMPING;
			dy = JUMP_FORCE;  // 초기 상승 속도
			float heightInMeters = GetCurrentHeight() / 10.0f;
			float baseJumpVelocity = -10.0f;

			// 속도 부스트 20m 이후 부터 속도 증가
			if (heightInMeters > 20.0f)
			{
				float speedBoostFactor = 1.0f + (heightInMeters - 20.0f) * 0.1f;
				// 과도한 가속 방지를 위해 속도 부스트 상한선 설정
				speedBoostFactor = min(speedBoostFactor, 2.0f);
				dy = baseJumpVelocity * speedBoostFactor;
			}
			else {
				dy = baseJumpVelocity;
			}

			jumpSound.play();
		}
	}
	// 배경 전환 시 낙하속도 제한
	void ResetJumpState()
	{
		// 점프 상태 초기화
		currentState = FALLING;
		dy = 0.5f; // 약간의 낙하 속도 부여
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
		return y > view.getCenter().y + HEIGHT / 2 + imgHeight; // 화면 아래로 완전히 벗어남
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

//const float Player::MAX_FALL_TIME = 2.0f; // 2초 동안 떨어진 후 제거

// 점프 bar 클래스
struct Pos		// bar의 위치 구조체
{
	int x;
	int y;
};

class Bar
{
public:
	vector<Pos> vBar;	// bar의 위치 정보 저장
	Sprite* imgBar;		// bar 이미지 나타냄 (sprite 객체)
	Texture t;			// bar 이미지 로드
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

		// 화면을 균등한 구역으로 나누기
		int sectionWidth = WIDTH / BAR_COUNT;

		// 초기 바 위치 생성
		int currentY = HEIGHT;
		for (int i = 0; i < BAR_COUNT; ++i)
		{
			Pos p;
			// 각 섹션 내에서 랜덤한 X 좌표 생성
			p.x = (WIDTH / 2) - (imgWidth / 2) + (rand() % (sectionWidth - imgWidth));

			// 랜덤 Y 좌표 간격을 설정
			p.y = currentY + rand() % (MAX_GAP - MIN_GAP + 1) - MIN_GAP;			// 최소/최대 간격 Y 좌표
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

	// cloud와 player의 충돌 여부
	bool CheckCollision(Player* pPlayer)
	{
		for (const auto& bar : vBar)
		{
			if (pPlayer->CheckCollision(bar.x, bar.y, imgWidth, imgHeight))
			{
				pPlayer->SetY(bar.y - pPlayer->GetHeight());
				pPlayer->Jump();
				return true; // 충돌 발생
			}
		}
		return false;
	}

	// 플랫폼 위치 갱신
	void MoveAndReset(Player* pPlayer)
	{
		for (auto& bar : vBar)
		{
			if (bar.y > pPlayer->GetY() + HEIGHT) // 화면 밖으로 벗어난 경우
			{
				bar.y -= TOTAL_HEIGHT;           // 플랫폼 재사용
				bar.x = (WIDTH / 2) - (imgWidth / 2) + (rand() % (WIDTH - imgWidth)); // 중앙 기준으로 새로운 X 좌표 계산
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


	// 게임 오버 관련 설정 추가
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
			// 게임 로직
			pPlayer->Move();
			pBar->CheckCollision(pPlayer);
			pBar->MoveAndReset(pPlayer);
			pPlayer->SetPosition();

			float currentHeight = pPlayer->GetCurrentHeight();
			background.update(currentHeight);

			// 콘솔에 현재 높이 출력 (0.5초마다)
			if (currentHeight - lastConsolePrintY >= 50) // 50cm마다 출력
			{
				std::cout << "Height: " << currentHeight / 10.0f << " meters" << std::endl;
				lastConsolePrintY = currentHeight;
			}

			//배경 전환 체크
			if (currentHeight - lastBackgroundChangeHeight >= 1000)
			{
				background.startFadeToNextBackground();
				lastBackgroundChangeHeight = currentHeight;
				std::cout << "Background changed at " << currentHeight / 10.0f << " meters" << std::endl;
				// 배경 전환 시 잠시 점프 방지
				pPlayer->ResetJumpState();
			}
			background.update(currentHeight);

			// 화면 따라가기
			view.setCenter(WIDTH / 2, pPlayer->GetY() + HEIGHT / 3);

			// 게임 오버 조건 확인
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

			// 화면 그리기
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
			// 게임 오버 상태에서 2초 후 창 닫기
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



