import json
import socket

class client:
    s:socket
    def __init__(self,host, port):
         self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
         self.s.connect((host, port))

    def __del__(self):
        self.s.close()

    def send_recv(self, request):
        self.s.sendall(request)
        response = self.s.recv(1024)
        print(f"<Recv>: {json.loads(response)!r}")
        print(f"<Send>: {json.loads(request)!r}")
        return response

cl = client("192.168.5.75", 7778)

# запрос температуры и напряжений
ret = cl.send_recv(b"{\"device\": 0,\"instance\": 0,\"command\": \"BRDctrl_SYSMON\"}")

# запись и чтение косвенного регистра
ret = cl.send_recv(b"{\"device\": 0,\"instance\": 0,\"command\": \"BRDctrl_REG_WRITEIND\",\"tetr\":1, \"reg\":21, \"val\":2}")
ret = cl.send_recv(b"{\"device\": 0,\"instance\": 0,\"command\": \"BRDctrl_REG_READIND\",\"tetr\":1, \"reg\":21}")

# запись и чтение прямого регистра
ret = cl.send_recv(b"{\"device\": 0,\"instance\": 0,\"command\": \"BRDctrl_REG_READDIR\",\"tetr\":0, \"reg\":0}")
ret = cl.send_recv(b"{\"device\": 0,\"instance\": 0,\"command\": \"BRDctrl_REG_WRITEDIR\",\"tetr\":0, \"reg\":0, \"val\":0}")

# запись чтение SPD регистра
ret = cl.send_recv(b"{\"device\": 0,\"instance\": 0,\"command\": \"BRDctrl_REG_WRITESPD\",\"tetr\":1, \"dev\":4, \"num\":8, \"reg\":0, \"val\":2}")
ret = cl.send_recv(b"{\"device\": 0,\"instance\": 0,\"command\": \"BRDctrl_REG_READSPD\",\"tetr\":1, \"dev\":4, \"num\":8, \"reg\":0}")

####################################
# командный интерфейс
####################################

# смена устройства, по умолчанию 0 (см. brd.ini)
#ret = cl.send_recv(b"{\"i-com\": \"l\", \"param\": "0"}") # 
# смена сервиса (новая ф-ция изменения)
#ret = cl.send_recv(b"{\"i-com\": \"s\", \"param\": \"REG0\"}") 
# Info
#ret = cl.send_recv(b"{\"i-com\": \"i\"}") 

####################################
# сервисный интерфейс доступа к регистрам
####################################
# spd доступ в этой части не реализован, он есть в общей части

# чтение/запись в регистры
ret = cl.send_recv(b"{\"s-reg\": \"4:0x100\"}")
ret = cl.send_recv(b"{\"s-reg\": \"4:0x400=0xABCD\"}")

####################################
# DAC
####################################
# конфигурация
ret = cl.send_recv(b"{\"dac\": \"config\",\"path\": \"exam_edac.ini\"}")
# работа
# mode - режим, если не задаётся, то определяется в exam_edac.ini, раздел [Option] в параметре WorkMode
#WorkMode    = 5   ; Режим теста
# 0 - однократный вывод с помощью ЦПУ через FIFO (в программном цикле)
# 1 - однократный вывод с помощью ПДП через FIFO (в программном цикле)
# 2 - однократный вывод с помощью ПДП через SDRAM (в программном цикле)
# 3 - вывод из FIFO в режиме рестарта
# 4 - вывод из SDRAM в режиме рестарта --- Только для независимых плат! ----
# 5 - циклический вывод из FIFO
# 6 - непрерывный вывод помощью ПДП через FIFO
# 7 - циклический вывод из SDRAM
ret = cl.send_recv(b"{\"dac\": \"work\",\"mode\": 1}")
ret = cl.send_recv(b"{\"dac\": \"work\"}")
# освободить занятые ресурсы
ret = cl.send_recv(b"{\"dac\": \"release\"}")

####################################
# ADC
####################################
# конфигурация
#ret = cl.send_recv(b"{\"adc\": \"config\",\"path\": \"exam_adc.ini3gda.ini\"}")
# работа
