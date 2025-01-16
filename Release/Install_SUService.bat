@echo off
sc delete AIAgentService
sc stop AIAgentService
mkdir "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\config.ini" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\dbproxy.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\dbproxy.lib" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\jsoncpp.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\jsoncpp.lib" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libiconv-2.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libintl-9.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libmysql.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libpq.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libpq.lib" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libssl-3-x64.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libcrypto-3-x64.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libwinpthread-1.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\msado60_backcompat.tlh" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\msado60_backcompat.tli" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\AIAgentService.exe" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\server.crt" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\server.csr" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\server.key" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\dh512.pem" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libssl-1_1-x64.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libcrypto-1_1-x64.dll" "C:\Program Files\AIAgentService"
xcopy /s /y "%~dp0\libboost_locale-vc143-mt-gd-x64-1_87.lib" "C:\Program Files\AIAgentService"
sc create AIAgentService binpath= "C:\Program Files\AIAgentService\AIAgentService.exe" start= auto
sc start AIAgentService

