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

// 플레이어 상태 열거형 정의
enum PlayerState { READY, JUMPING, FALLING };

// 배경화면 class
class Background
{
private:
	Texture backgroundTexture;	// 배경 텍스처
	Sprite backgroundSprite;	// 배경 스프라이트
	float currentHeight;		// 현재 높이
	float fadeAlpha;			// 페이드 효과 알파값
	bool isFading;				// 페이드 상태 플래그
	float totalHeight;			// 페이드 상태 플래그
	bool barStateChanged;		//bar 상태 변경

public:
	// 생성자: 배경 이미지 파일 경로를 받아 초기화
	Background(const std::string& filePath)
	{
		// 긴 배경 이미지 로드
		if (!backgroundTexture.loadFromFile(filePath)) {
			throw std::runtime_error("Failed to load texture: " + filePath);
		}

		backgroundSprite.setTexture(backgroundTexture);

		// 초기 값 설정
		currentHeight = 0;
		totalHeight = backgroundSprite.getLocalBounds().height * backgroundSprite.getScale().y;
		barStateChanged = false;
	}

	// 배경 업데이트 함수
	void update(float currentHeight, float deltaTime) 
	{
		// 스크롤 속도 조정 (예: 100픽셀/초)
		float scrollSpeed = 100.0f;
		currentHeight += scrollSpeed * deltaTime;

		// 배경이 끝나면 다시 시작. 이거 없으면 배경 생성 이상해짐
		if (currentHeight >= totalHeight) {
			currentHeight = 0;
			barStateChanged = false; // 배경 루프 시 bar 상태 초기화
		}

		// 배경 위치 업데이트
		backgroundSprite.setPosition(0, -fmod(currentHeight, totalHeight));

		// y 좌표가 150 이상일 때 bar 상태 변경
		if (currentHeight >= 150 && !barStateChanged) {
			barStateChanged = true;
			changeBarState(); // bar 상태를 변경하는 함수 호출
		}
	}
	// 배경 그리기
	void draw(RenderWindow& window)
	{
		window.draw(backgroundSprite);
	}
	// Bar 상태 변경 함수
	void changeBarState()
	{
		std::cout << "Bar 상태가 변경되었습니다!" << std::endl;
	}

	// 다음 배경으로 페이드 효과 
//	void startFadeToNextBackground()
//	{
//		isFading = true;
//		fadeAlpha = 255;
//	}

};


// 플레이어 클래스
class Player
{
private:
	int x, y;				// 플레이어 위치
	int imgWidth, imgHeight;// 플레이어 이미지 크기
	float dy;				// 수직 이동 속도
	Sprite* imgJump;		// 점프 중 이미지
	Sprite* imgReady;		// 대기 중 이미지

	PlayerState currentState;	// 현재 상태
		
	Texture t1, t2;

	// 점프 sound
	sf::SoundBuffer jumpSoundBuffer;
	sf::Sound jumpSound;

	const float GRAVITY = 0.5f;      // 중력
	const float JUMP_FORCE = -15.0f; // 점프 초기 속도

private:
	// 상태에 따른 이미지 변환
	const Sprite& GetImg()		// 점프 중인 이미지 반환
	{
		return (currentState == JUMPING) ? *imgJump : *imgReady;
	}

public:
	// 생성자 : 기본 갑 초기화
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

	// 플레이어 위치 설정
	void SetPosition()
	{
		imgReady->setPosition(x, y);
		imgJump->setPosition(x, y);
	}
	// 현재 높이 반환
	float GetCurrentHeight() const {
		return TOTAL_HEIGHT - y;
	}
	// 플레이어 이동 처리
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

		// 화면 경계
		if (x < 0)	//왼쪽 벽 뚫지 못하게
		{
			x = 0;
		}
		if (x > WIDTH - imgWidth)	//오른쪽 벽 뚫지 못하게
		{
			x = WIDTH - imgWidth;
		}

		// 중력 작용
		dy += GRAVITY;
		y += static_cast<int>(dy);

		// 상태 업데이트
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

	// 화면에 이미지 그리기
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

	// 점프 시작
	void Jump()
	{
		// 현재 상태가 READY(준비) 또는 FALLING(낙하 중)일 때만 점프 가능
		if (currentState == READY || currentState == FALLING)
		{
			currentState = JUMPING;	// 점프 상태로 변경
			dy = JUMP_FORCE;		// 초기 상승 속도
			float heightInMeters = GetCurrentHeight() / 10.0f;		// 현재 높이를 미터 단위로 변환
			float baseJumpVelocity = -10.0f;	// 기본 점프 속도

			// 속도 부스트 20m 이후 부터 속도 증가
			if (heightInMeters > 20.0f)
			{
				// 높이에 따른 속도 증가
				float speedBoostFactor = 1.0f + (heightInMeters - 20.0f) * 0.1f;
				// 과도한 가속 방지를 위해 속도 부스트 상한선 설정
				speedBoostFactor = min(speedBoostFactor, 2.0f);
				dy = baseJumpVelocity * speedBoostFactor;
			}
			else 
			{
				dy = baseJumpVelocity;		// 기본 점프 속도 사용
			}

			jumpSound.play();		
		}
	}
	// 착지 처리 함수
	void Land() {
		currentState = READY;	// READY 상태로 변경
		dy = 0;					// 수직 속도 초기화
	}
	// 배경 전환 시 낙하속도 제한
	void ResetJumpState()
	{
		currentState = FALLING;	// FALLING 상태로 변경
		dy = 0.5f;				// 약간의 낙하 속도 부여
	}

	// 플레이어와 장애물(bar) 충돌 체크
	bool CheckCollision(int barX, int barY, int barWidth, int barHeight)
	{
		return (x < barX + imgWidth * 0.15f &&	// x축 충돌
			x + imgWidth > barX &&				
			y + imgHeight >= barY &&			// Y축 충돌
			y + imgHeight <= barY + imgHeight * 0.15f &&
			dy > 0);							// 플레이어가 아래로 이동 중
	}
	// 화면 밖으로 벗어났는지 확인
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

// 점프 bar 클래스
struct Pos		// bar의 위치 구조체
{
	int x;
	int y;
};

class Bar
{
//private:
//	RectangleShape barShape; // 진행도를 나타내는 바
//	float maxWidth;          // 바의 최대 너비
//	float currentProgress;   // 현재 진행도 (0 ~ 100)
//	Color normalColor;
//	Color activeColor;

public:
	vector<Pos> vBar;	// bar의 위치 정보 저장
	Sprite* imgBar;		// bar 이미지 나타냄 (sprite 객체)
	Texture t;			// bar 이미지 로드
	Texture t2, t3, t4;	// 다른 bar 텍스처들
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

			vBar.push_back(p);	// 위치 정보를 벡터에 저장
		}
	}
	~Bar()
	{
		delete(imgBar);
	}

	// bar 그리기
	void Draw(RenderWindow& window)
	{
		for (const auto& bar : vBar)
		{
			imgBar->setPosition(bar.x, bar.y);
			window.draw(*imgBar);
		}
	}

	// bar의 y 좌표가 특정 값을 초과했을 때 텍스처 변경
	void UpdateTextures()
	{
		// 텍스처 변경 순서를 추적하는 변수
		static int textureIndex = 0;

		for (auto& bar : vBar)
		{
			if (bar.y > 145) // y 좌표가 145를 초과하면 텍스처 변경
			{
				switch (textureIndex)
				{
					case 0: imgBar->setTexture(t); break;
					case 1: imgBar->setTexture(t2); break;
					case 2: imgBar->setTexture(t3); break;
					case 3: imgBar->setTexture(t4); break;
					default: imgBar->setTexture(t); break;
				}
				// 텍스처 인덱스를 순차적으로 증가시키고 4가 되면 다시 0으로 설정
				textureIndex = (textureIndex + 1) % 4;

				// 해당 bar의 y 좌표를 초기화 또는 새로운 위치로 이동
				bar.y = rand() % 100; // 임의로 재설정
			}
		}
	}

	// cloud와 player의 충돌 여부
	bool CheckCollision(Player* pPlayer)
	{
		for (const auto& bar : vBar) 
		{
			if (pPlayer->CheckCollision(bar.x, bar.y, imgWidth, imgHeight)) 
			{
				pPlayer->SetY(bar.y - pPlayer->GetHeight());	// 충돌 시 플레이어 위치 조정
				pPlayer->Land();
				return true;
			}
		}
		return false;
	}

	// bar 위치 갱신
	void MoveAndReset(Player* pPlayer)
	{
		for (auto& bar : vBar)
		{
			if (bar.y > pPlayer->GetY() + HEIGHT) // 화면 밖으로 벗어난 경우
			{
				bar.y -= TOTAL_HEIGHT;           // bar 위치를 위로 이동
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

	Player player;
	Bar barSystem;

	Background background("assets/images/backgrounds.png");

	// 높이 관련 변수 초기화
	float lastBackgroundChangeY = 0;
	float lastConsolePrintY = 0;
	float lastBackgroundChangeHeight = 0;
	float currentHeight;


	// 게임 오버 관련 설정 추가
	sf::Clock fallClock;	// 낙하 시간
	sf::Clock deltaClock;	// 시간 간격
	sf::Font font;

	// game over
	sf::Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setString("Game Over!");
	gameOverText.setCharacterSize(50);
	gameOverText.setFillColor(Color::Red);
	gameOverText.setPosition(WIDTH / 2 - 100, HEIGHT / 2);

	// 높이 표시 텍스트 초기화
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
			if (pBar->CheckCollision(pPlayer)) {
				pPlayer->Jump();
			};
			barSystem.MoveAndReset(&player); // bar 위치 갱신
			barSystem.UpdateTextures();     // 텍스처 업데이트

			// 화면 초기화
			window.clear();
			barSystem.Draw(window);         // bar 그리기
			player.Draw(window);            // 플레이어 그리기
			window.display();
			pPlayer->SetPosition();			// 플레이어 위치 설정

			float currentHeight = pPlayer->GetCurrentHeight();		// 현재 높이 가져오기
			background.update(currentHeight, deltaTime);			// 배경 업데이트

			// 콘솔에 현재 높이 출력
			if (currentHeight - lastConsolePrintY >= 1000) // 10m마다 출력
			{
				std::cout << "Height: " << currentHeight / 10.0f << " meters" << std::endl;
				lastConsolePrintY = currentHeight;
			}

			// 화면 따라가기
			view.setCenter(WIDTH / 2, pPlayer->GetY() + HEIGHT / 3);

			// 게임 오버 조건 확인
			if (pPlayer->IsOutOfScreen(view))
			{
				gameOver = true;
				fallClock.restart();
			}

			currentHeight = pPlayer->GetCurrentHeight();
			if (currentHeight > maxHeight)	// 최대 높이 기록 갱신
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