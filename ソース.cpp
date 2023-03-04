#include <iostream>
#include <vector>
#include <cstdint>
#include <tuple>
#include <string>

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
class TCPClient {
public:
	TCPClient() {}
	TCPClient(unsigned long IP, u_short Port) {
		Connect(IP, Port);
		return;
	}

	virtual ~TCPClient() {
		if (IsRunning()) { DisConnect(); }
		return;
	}
	bool IsRunning() {
		return S != INVALID_SOCKET;
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

	int Write(const std::vector<char>& In) {
		return send(S, In.data(), In.size(), 0);
	}

	int Wrire(const char* Data, std::size_t L) {
		return send(S, Data, L, 0);
	}

	std::vector<char> Read() {

		static const int L = 65000;
		char D[L] = { 0, };

		int SA = sizeof(SA);

		int R = 0;

		std::vector<char> RR;


		do {
			R = recv(S, D, L, 0);
			if (R != SOCKET_ERROR) { RR.insert(RR.end(), D, D + R); }//min provide by windows. yes i know STL Have too.but it is confrict. 
			auto a = WSAGetLastError();
		} while (R != SOCKET_ERROR && R != 0);
		//while (R = recvfrom(S, D, L, 0, (SOCKADDR*)&A, &SA)) {



		return RR;
	}
	int Read(char* Buf, std::size_t L) {
		int SA = sizeof(SA);
		return recv(S, Buf, L, 0);
		//return recvfrom(S, Buf, L, 0, (SOCKADDR*)&A, &SA);
	}

	SOCKADDR_IN GetSockAdder() {
		return SA;
	}

	int DisConnect() {
		int A = shutdown(S, SD_BOTH);
		if (A == SOCKET_ERROR) { return A; }
		int B = closesocket(S);
		if (B != SOCKET_ERROR) { S = INVALID_SOCKET; }
		SA = SOCKADDR_IN{ 0, };
		return B;
	}

protected:
	SOCKET S = INVALID_SOCKET;
	SOCKADDR_IN SA = { 0, };
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

int main() {
	WinSockCaller WS;
	WS.Call();
	const char *HN1 = "www.yahoo.co.jp";
	const char* HN2 = "www.google.co.jp";
	const char* HN3 = "www.microsoft.co.jp";
	auto IP=GetIPByName(HN2);//this is not perfect. ex.microsoft.

	//auto A = GetHostByAddr(SepIP(std::get<1>(IP[0])));
	

	TCPClient TCP(std::get<1>(IP[0]), 23);
	auto A=WSAGetLastError();

	if (!TCP.IsRunning()) { return -1; }
	auto X = TCP.Read();
	const char M[] = "Hello world!";
	TCP.Wrire(M, sizeof(M));

	return 0;
	
}