#include <iostream>
#include <vector>
#include <cstdint>
#include <tuple>
#include <string>
#include <future>
#include <deque>
#include <algorithm>

#include <conio.h>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable : 4996)

class WinSockCaller {
public:
	int Call() {
		return WSAStartup(MAKEWORD(2, 2), &WD);
	}
	virtual ~WinSockCaller() {
		Close();
	}

	int Close() {
		return WSACleanup();
	}

	WSADATA GetData() {
		return WD;
	}


protected:
	WSADATA WD = { 0, };
};
class TCPServer {
public:
	TCPServer() {}
	TCPServer(unsigned long IP, u_short Port) {
		Connect(IP, Port);
		return;
	}

	virtual ~TCPServer() {
		if (IsRunning()) { DisConnect(); }
		return;
	}
	bool IsRunning() {
		return S != INVALID_SOCKET;
	}

	

	SOCKET GetSocket() {
		return S;
	}

	int Connect(unsigned long IP, unsigned short Port) {
		S = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (S == INVALID_SOCKET) { return INVALID_SOCKET; }

		SOCKADDR_IN SI = { 0 , };

		SI.sin_family = AF_INET;
		SI.sin_addr.S_un.S_addr = IP;
		SI.sin_port = htons(Port);

		int A = bind(S, (SOCKADDR*)&SI, sizeof(SI));
		if (A == SOCKET_ERROR) { closesocket(S); S = INVALID_SOCKET; return SOCKET_ERROR; }

		int B = listen(S, SOMAXCONN);
		if (B == SOCKET_ERROR) { closesocket(S); S = INVALID_SOCKET; return SOCKET_ERROR; }
		SA = SI;
		return 0;
	}

	int Write(const std::vector<char>& In,std::size_t Pos) {
		return send(SS[Pos], In.data(), In.size(), 0);
	}

	int Wrire(const char* Data, std::size_t L,std::size_t Pos) {
		return send(SS[Pos], Data, L, 0);
	}

	std::vector<char> Read(std::size_t Pos) {

		static const int L = 65000;
		char D[L] = { 0, };

		int SA = sizeof(SA);

		int R = 0;

		std::vector<char> RR;


		do {
			R = recv(SS[Pos], D, L, 0);
			if (R != SOCKET_ERROR) { RR.insert(RR.end(), D, D + R); }//min provide by windows. yes i know STL Have too.but it is confrict. 
			auto a = WSAGetLastError();
		} while (R != SOCKET_ERROR && R != 0);
		//while (R = recvfrom(S, D, L, 0, (SOCKADDR*)&A, &SA)) {



		return RR;
	}
	int Read(char* Buf, std::size_t L,std::size_t Pos) {
		int SA = sizeof(SA);
		return recv(SS[Pos], Buf, L, 0);
		//return recvfrom(S, Buf, L, 0, (SOCKADDR*)&A, &SA);
	}

	int Accept() {
		SOCKET X = accept(S, nullptr, nullptr);//this is blocking function. i need async it.
		if (X != INVALID_SOCKET) { SS.push_back(X); }
		return X;
	}

	SOCKADDR_IN GetSockAdder() {
		return SA;
	}

	std::size_t Size() { return SS.size(); }

	int DisConnect() {
		int A = shutdown(S, SD_BOTH);
		if (A == SOCKET_ERROR) { return A; }
		int B = closesocket(S);
		if (B != SOCKET_ERROR) { S = INVALID_SOCKET; }
		SA = SOCKADDR_IN{ 0, };
		return B;
	}

	std::vector<SOCKET>::iterator begin() {
		return SS.begin();
	}
	std::vector<SOCKET>::iterator end() {
		return SS.end();
	}
	SOCKET operator[](std::size_t In) {
		return SS[In];
	}

protected:
	SOCKET S = INVALID_SOCKET;
	SOCKADDR_IN SA = { 0, };
	std::vector<SOCKET> SS;
};

unsigned long IPByNumber(unsigned char A, unsigned char B, unsigned char C, unsigned char D) {
	return D << 24 | C << 16 | B << 8 | A;
}

SOCKADDR_IN MakeSOCKADDER(short F, unsigned char A, unsigned char B, unsigned char C, unsigned char D, unsigned short Port) {
	SOCKADDR_IN R;
	R.sin_family = F;
	R.sin_port = htons(Port);
	R.sin_addr.S_un.S_addr = IPByNumber(A, B, C, D);

	return R;
}

std::vector<std::tuple<std::string,unsigned long>>  GetIPByName(const char* Name) {
	LPHOSTENT LHT = gethostbyname(Name);
	if (LHT == nullptr)return{};
	if (LHT->h_addrtype != AF_INET) { return {}; }

	std::vector<std::tuple<std::string,unsigned long>> IPs;

	SOCKADDR_IN A;
	
	for (std::size_t i = 0; LHT->h_addr_list[i]; i++) {
		A.sin_addr.S_un.S_addr = *(u_long*)LHT->h_addr_list[i];

		IPs.push_back({ {(LHT->h_addr_list[i])}, A.sin_addr.S_un.S_addr});
	}
	
		return IPs;
}

std::string SepIP(u_long In) {
	std::string R;

	R = std::to_string(In>>24 & 0xff)+',';
	R += std::to_string(In>>16 & 0xff)+',';
	R += std::to_string(In>>8 & 0xff)+',';
	R += std::to_string(In & 0xff);

	return R;
}

std::string GetHostByAddr(std::string Addr) {
	LPHOSTENT LHT = gethostbyaddr(Addr.c_str(),4,AF_INET);
	if (LHT == nullptr) { return {}; }
	return LHT->h_name;
}

int KeyIn() {

	int K = 0;

	if (_kbhit()!=0) {
		K = _getch();
	}

	return K;
}

int main() {
	WinSockCaller WC;
	WC.Call();

	const char* HN4 = "localhost.";
	u_short Po = 27015;

	auto IP = GetIPByName(HN4);

	std::cout << "Start Setup." << std::endl;

	TCPServer TS;
	auto A = TS.Connect(std::get<1>(IP[0]), Po);

	if (A == SOCKET_ERROR) { return -1; }

	std::cout << "Start Loop." << std::endl;

	bool F = true;

	while (TS.IsRunning()) {
		if (F) { 
			int D = TS.Accept();//we need async this.
			if (D != SOCKET_ERROR) {	std::cout << "I Got New Connection." << std::endl;}
			F = false;
		}
		
		for (std::size_t i = 0; i < TS.Size(); i++) {
			const size_t L = (1 << 15) - 1;
			char B[L] = { 0, };

			int X = recv(TS[i], B, L, 0);
			int Y = min(X, L);
			B[Y] = '\0';
			std::cout << "InComing:" << B << std::endl;
			int Z = send(TS[i], B, Y, 0);//where am i send to...????
			std::cout << "Done Send : " <<Z<< std::endl;
		}
		int K = KeyIn();
		if (K == ' ') { 
			F = true; 
			std::cout << "Start Waiting Accept." << std::endl;
		}
		if (K == 27) {
			std::cout << "End App Loop." << std::endl;
			break;
		}
	}

	return 0;

}

/** /
int main() {
	WinSockCaller WS;
	WS.Call();

	const char *HN1 = "www.yahoo.co.jp";
	const char* HN2 = "www.google.co.jp";
	const char* HN3 = "www.microsoft.co.jp";
	const char* HN4 = "localhost.";
	u_short Po = 27015;

	auto IP=GetIPByName(HN4);//this is not perfect. ex.microsoft.

	//auto A = GetHostByAddr(SepIP(std::get<1>(IP[0])));
	

	TCPServer TCP(std::get<1>(IP[0]), Po);
	auto A=WSAGetLastError();

	std::cout << "START TCP Server!" << std::endl;
	
	std::vector<SOCKET> SS;

	const std::size_t L = (1<<15)-1;
	char C[L] = {0,};

	std::cout << "Start Loop!" << std::endl;
	std::deque<SOCKET> SQ;
	while (TCP.IsRunning()) {
		auto F = std::async(std::launch::deferred, [&](){while (TCP.IsRunning()) { SQ.push_back(accept(TCP.GetSocket(), nullptr, nullptr)); }});
		if (F.wait_for(std::chrono::seconds(1))!=std::future_status::timeout) { F.wait(); };
		for (auto o : SQ) {
			if (o != INVALID_SOCKET) { SS.push_back(o); }
		}
		SQ.clear();

		std::cout <<"Size:" << SS.size() << '\r';
		for (auto o : SS) {
			int LL = recv(o, C, L, 0);
			if (LL != SOCKET_ERROR) {
				C[LL] = '\0';
				std::cout << C << std::endl;
				send(o, C, LL, 0);
			}
		}
		std::cout << "Running!" << std::endl;
		if (KeyIn() == 27) { break; }
	}
	//for (auto o : SS) { closesocket(o); }
	return 0;
	
}
/**/