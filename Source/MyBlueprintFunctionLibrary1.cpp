// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary1.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include <cstdio>
#include<iostream>
#include<string>
#include "Sockets.h"
#include "SocketSubsystem.h"

using namespace std;
const int PORT = 4844;
#define MaxBufSize 1024


FString UMyBlueprintFunctionLibrary1::Connected(FString str)
{
    std::string sstr = TCHAR_TO_UTF8(*str);
    if (sstr.length()<4)
        return FString("fail");
        
    FSocket* ClientSocket;
    FString inip = TEXT("159.75.219.151");
    int32 port = 4844;
    FString ClientSocketName = FString("unreal socket");
    int BufferMaxSize = 1024;
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    auto ResolveInfo = SocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*inip));
    while (!ResolveInfo->IsComplete());
    auto error = ResolveInfo->GetErrorCode();

    auto RemoteAdress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    RemoteAdress->SetRawIp(ResolveInfo->GetResolvedAddress().GetRawIp()); // todo: somewhat wasteful, we could probably use the same address object?
    RemoteAdress->SetPort(port);

    ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);

    //Set Send Buffer Size
    ClientSocket->SetSendBufferSize(BufferMaxSize, BufferMaxSize);
    ClientSocket->SetReceiveBufferSize(BufferMaxSize, BufferMaxSize);
    ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);


    uint32 BufferSize = 1024;
    TArray<uint8> ReceiveBuffer;
    TArray<uint8> SendBuffer;
    TCHAR* seriallizedChar = str.GetCharArray().GetData();
    ClientSocket->Connect(*RemoteAdress);
    int32 sizee = 0;
    ClientSocket->Send((uint8*)TCHAR_TO_UTF8(seriallizedChar), FCString::Strlen(seriallizedChar) + 1, sizee);

    int32 readd = 0;
    uint8 element = 0;
    ReceiveBuffer.Init(element, 1024u);
    ClientSocket->Recv(ReceiveBuffer.GetData(), ReceiveBuffer.Num(), readd);
    const FString ReceivedUE4String = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceiveBuffer.GetData())));
    return ReceivedUE4String;

}

void UMyBlueprintFunctionLibrary1::DisConnected(FString str)
{
	FSocket* ClientSocket;
	str = FString("kill") + str;
	FString inip = TEXT("159.75.219.151");
	int32 port = 4844;
	FString ClientSocketName = FString("unreal socket");
	int BufferMaxSize = 1024;
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	auto ResolveInfo = SocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*inip));
	while (!ResolveInfo->IsComplete());
	auto error = ResolveInfo->GetErrorCode();

	auto RemoteAdress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	RemoteAdress->SetRawIp(ResolveInfo->GetResolvedAddress().GetRawIp()); // todo: somewhat wasteful, we could probably use the same address object?
	RemoteAdress->SetPort(port);

	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);

	//Set Send Buffer Size
	ClientSocket->SetSendBufferSize(BufferMaxSize, BufferMaxSize);
	ClientSocket->SetReceiveBufferSize(BufferMaxSize, BufferMaxSize);
	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);


	uint32 BufferSize = 1024;
	TArray<uint8> ReceiveBuffer;
	TArray<uint8> SendBuffer;

	TCHAR* seriallizedChar = str.GetCharArray().GetData();
	ClientSocket->Connect(*RemoteAdress);
	int32 sizee = 0;
	ClientSocket->Send((uint8*)TCHAR_TO_UTF8(seriallizedChar), FCString::Strlen(seriallizedChar) + 1, sizee);

}