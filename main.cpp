#include <stdio.h>
#include <curl/curl.h>
#include"cJSON.h"

struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);

    if (ptr == NULL) {
        printf("realloc() failed\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char* SendJson(cJSON* json)
{
    CURL* curl;
    CURLcode res;
    struct MemoryStruct chunk = { 0 };
    char* printstr = cJSON_Print(json);
    char* result = NULL;
    //printf("json data:%s\n", printstr);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        // 查看请求头
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        // 设置要请求的 URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        // 设置请求头信息
        curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Authorization: Bearer $CHATGPT_API_KEY$");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // Authorization: Bearer $OPENAI_API_KEY

        // 添加 POST 数据
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, printstr);

        // 执行请求
        res = curl_easy_perform(curl);  // 请求成功这里会打印消息

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        }
        else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            //printf("HTTP response code: %ld\n", http_code);
            //printf("chunk memory:%s\n", chunk.memory);
            cJSON* response = cJSON_Parse(chunk.memory);
            cJSON* choices = cJSON_GetObjectItem(response, "choices");
            cJSON* Msg = cJSON_GetArrayItem(choices, 0);
            cJSON* item_0 = cJSON_GetObjectItem(Msg, "message");
            cJSON* item_1 = cJSON_GetObjectItem(item_0, "content");
            result = cJSON_Print(item_0);

            int len= MultiByteToWideChar(CP_UTF8, 0, item_1->valuestring, -1, NULL, 0);
            wchar_t* transform = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, item_1->valuestring, -1, transform, len);
            len = WideCharToMultiByte(CP_ACP, 0, transform, -1, NULL, 0, NULL, NULL);
            char* show = new char[len];
            WideCharToMultiByte(CP_ACP, 0, transform, -1, show, len, NULL, NULL);
            printf("answer:%s\n", show);
            delete[] transform;
            delete[] show;

            cJSON_Delete(response);
        }

        // 清理 curl
        curl_easy_cleanup(curl);
    }
    if (chunk.memory != NULL)
        free(chunk.memory);
    if (printstr != NULL)
        free(printstr);
    return result;
}

int main(int argc, char* argv[])
{
    cJSON* json = cJSON_Parse("{\"model\": \"gpt-3.5-turbo\"}");
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "messages", array);
    while (true)
    {
        char buf[128] = "";
        wchar_t transform[128] = L"";
        printf("我的发言为:");
        gets_s(buf);
        if (strcmp(buf, "quit") == 0)break;
        MultiByteToWideChar(CP_ACP, 0, buf, -1, transform, 128);
        WideCharToMultiByte(CP_UTF8, 0, transform, -1, buf, 128, NULL, NULL);
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "role", "user");
        cJSON_AddStringToObject(item, "content", buf);
        cJSON_AddItemToArray(array, item);
        char* answer = SendJson(json);
        if (answer != NULL)
        {
            //printf("answer json:%s\n", answer);
            cJSON_AddItemToArray(array, cJSON_Parse(answer));
        }
        else
        {
            cJSON_DeleteItemFromArray(item, cJSON_GetArraySize(array) - 1);
        }
    }
    cJSON_Delete(json);
    return 0;
}
