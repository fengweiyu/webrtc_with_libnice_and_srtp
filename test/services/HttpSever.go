package services

import (
	"fmt"
	"net/http"
	"os/exec"
	"strconv"
	"time"
)

var testRes = 0
var testTalkRes = 0

// 函数名开头大写，表示可公开的
func TestResultHandler(w http.ResponseWriter, r *http.Request) {
	method := r.Method
	switch method {
	case "OPTIONS":
		{
			// 设置允许的请求方法
			w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
			// 设置允许的请求头
			w.Header().Set("Access-Control-Allow-Headers", "access-control-allow-headers,Access-Control-Allow-Origin,content-type")
			// 设置允许跨域请求的来源
			w.Header().Set("Access-Control-Allow-Origin", "*")
			// 处理请求
			w.WriteHeader(http.StatusOK)
		}
	case "POST":
		{
			res := r.URL.Query().Get("res")
			fmt.Println("Received res:", res)

			resTalk := r.URL.Query().Get("resTalk")
			fmt.Println("Received resTalk:", resTalk)

			if res == "1" {
				testRes++
			} else {
				testRes = 0
			}
			if resTalk == "1" {
				testTalkRes++
			} else {
				testTalkRes = 0
			}
			w.Header().Set("Access-Control-Allow-Origin", "*")
			w.WriteHeader(http.StatusOK)

		}
	case "GET":
		{
			fmt.Println("Received a GET request")
		}
	case "DELETE":
		fmt.Println("Received a DELETE request")
	default:
		fmt.Println("Received an unknown request method")
	}
}

var google_args = " --no-sandbox --disable-setuid-sandbox --use-fake-ui-for-media-stream --use-fake-device-for-media-stream --use-file-for-fake-audio-capture=C:/Users/Admin/AppData/Local/Google/Chrome/Application/2024.wav --app=file://C:/Users/Admin/AppData/Local/Google/Chrome/Application/testTalk.html"
var google_path = "C:/Users/Admin/AppData/Local/Google/Chrome/Application/chrome.exe"
var google_talk_args = " --use-fake-ui-for-media-stream --use-fake-device-for-media-stream --app=file://C:/Program Files (x86)/Google/Chrome/Application/testTalk.html"
var google_play_args = " --no-sandbox --disable-setuid-sandbox --app=file://C:/Program Files (x86)/Google/Chrome/Application/testCamera.html"

func TestHandler(w http.ResponseWriter, r *http.Request) {
	method := r.Method
	switch method {
	case "OPTIONS":
		{
			// 设置允许的请求方法
			w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
			// 设置允许的请求头
			w.Header().Set("Access-Control-Allow-Headers", "access-control-allow-headers,Access-Control-Allow-Origin,content-type")
			// 设置允许跨域请求的来源
			w.Header().Set("Access-Control-Allow-Origin", "*")
			// 处理请求
			w.WriteHeader(http.StatusOK)
		}
	case "POST":
		{
			w.Header().Set("Access-Control-Allow-Origin", "*")
			w.WriteHeader(http.StatusOK)

		}
	case "GET":
		{
			num := r.FormValue("num")
			intValue, err := strconv.Atoi(num)
			testRes = 0
			if err != nil {
				fmt.Println("转换失败:", err)
				cmdCamera := exec.Command(google_path, google_play_args)
				err = cmdCamera.Start()
				if err != nil {
					panic(err)
				}
			} else {
				for i := 0; i < intValue; i++ {
					cmdCamera := exec.Command(google_path, google_play_args) //"testCamera.html"
					err = cmdCamera.Start()
					if err != nil {
						panic(err)
					}
				}
			}

			cnt := 0
			for testRes != intValue {
				time.Sleep(1 * time.Second)
				cnt++
				if cnt > 15 {
					break
				}
			}
			if testRes == intValue {
				w.Header().Set("Access-Control-Allow-Origin", "*")
				w.WriteHeader(http.StatusOK)
			} else {
				w.Header().Set("Access-Control-Allow-Origin", "*")
				w.WriteHeader(http.StatusInternalServerError)
			}
		}
	case "DELETE":
		fmt.Println("Received a DELETE request")
	default:
		fmt.Println("Received an unknown request method")
	}
}

func TestTalkHandler(w http.ResponseWriter, r *http.Request) {
	method := r.Method
	switch method {
	case "OPTIONS":
		{
			// 设置允许的请求方法
			w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
			// 设置允许的请求头
			w.Header().Set("Access-Control-Allow-Headers", "access-control-allow-headers,Access-Control-Allow-Origin,content-type")
			// 设置允许跨域请求的来源
			w.Header().Set("Access-Control-Allow-Origin", "*")
			// 处理请求
			w.WriteHeader(http.StatusOK)
		}
	case "POST":
		{
			w.Header().Set("Access-Control-Allow-Origin", "*")
			w.WriteHeader(http.StatusOK)

		}
	case "GET":
		{
			num := r.FormValue("num")
			intValue, err := strconv.Atoi(num)
			testTalkRes = 0
			if err != nil {
				fmt.Println("转换失败:", err)
				cmdCamera := exec.Command(google_path, google_talk_args)
				err = cmdCamera.Start()
				if err != nil {
					panic(err)
				}
			} else {
				for i := 0; i < intValue; i++ {
					cmdCamera := exec.Command(google_path, google_talk_args)
					err = cmdCamera.Start()
					if err != nil {
						panic(err)
					}
				}
			}

			cnt := 0
			for testTalkRes != intValue {
				time.Sleep(1 * time.Second)
				cnt++
				if cnt > 15 {
					break
				}
			}
			if testTalkRes == intValue {
				w.Header().Set("Access-Control-Allow-Origin", "*")
				w.WriteHeader(http.StatusOK)
			} else {
				w.Header().Set("Access-Control-Allow-Origin", "*")
				w.WriteHeader(http.StatusInternalServerError)
			}
		}
	case "DELETE":
		fmt.Println("Received a DELETE request")
	default:
		fmt.Println("Received an unknown request method")
	}
}
