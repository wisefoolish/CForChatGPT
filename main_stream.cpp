#include <stdio.h>
#include <curl/curl.h>
#include"cJSON.h"

struct MemoryStruct {
    char* memory;
    size_t size;
};

// 解析传递过来的文件
char** MatchString(const char* str, int* result)
{
    int resultNum = 0;
    char** tempBuf = NULL;
    char* temp = (char*)malloc(sizeof(char) * strlen(str) + 1);
    if (temp == NULL)
    {
        *result = 0;
        return NULL;
    }
    memset(temp, 0, sizeof(char) * strlen(str) + 1);
    char match[] = "data: ";
    int index = 0;
    char flag = 0;
    int bracket = 0;
    char flag_bracket = 0;
    char flag_quotation = 0;
    char readyFor = 0;
    while (*str != '\0')
    {
        if (flag == 0)
        {
            if (*str == match[index])index++;
            else
            {
                index = 0;
                if (*str == match[index])index++;
            }
            if (index == strlen(match))
            {
                index = 0;
                flag = 1;
            }
        }
        else
        {
            temp[index++] = *str;
            if (!flag_quotation && *str == '{')
            {
                bracket++;
                flag_bracket = 1;
            }
            else if (!flag_quotation && *str == '}')bracket--;
            else if (!flag_quotation && !readyFor && *str == '\"')flag_quotation = 1;
            else if (flag_quotation && !readyFor && *str == '\"')flag_quotation = 0;
            if (!readyFor && *str == '\\')readyFor = 1;
            else readyFor = 0;
            if (flag_bracket && bracket == 0)
            {
                temp[index] = '\0';
                index = 0;
                flag_bracket = 0;
                flag = 0;
                char* item = (char*)malloc(sizeof(char) * (strlen(temp) + 1));
                strcpy_s(item, sizeof(char) * (strlen(temp) + 1), temp);
                char** anoArr = (char**)malloc(sizeof(char*) * (resultNum + 1));
                if (tempBuf != NULL)
                {
                    memcpy(anoArr, tempBuf, sizeof(char*) * resultNum);
                    free(tempBuf);
                }
                anoArr[resultNum] = item;
                resultNum++;
                tempBuf = anoArr;
            }
        }
        str++;
    }
    free(temp);
    *result = resultNum;
    return tempBuf;
}

void ClearJBuf(char** buf, int result)
{
    if (buf == NULL)return;
    for (int i = 0; i < result; i++)
    {
        if (buf[i] != NULL)
            free(buf[i]);
    }
    free(buf);
}

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    /*
    - `contents`：指向下载的数据缓冲区。
    - `_size`：缓冲区内每个数据块的大小，一般为1。
    - `nmemb`：缓冲区内数据块的个数。
    - `userp`：用户自定义数据指针，用于传递用户自定义的数据。在本函数中一般用不到，可以设置为NULL。
    */
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    int result = 0;
    char** json_s = MatchString((char*)contents, &result);
    for (int i = 0; i < result; i++)
    {
        cJSON* response = cJSON_Parse(json_s[i]);
        cJSON* choices = cJSON_GetObjectItem(response, "choices");
        cJSON* Msg = cJSON_GetArrayItem(choices, 0);
        cJSON* item_0 = cJSON_GetObjectItem(Msg, "delta");
        cJSON* item_1 = cJSON_GetObjectItem(item_0, "content");
        if (item_1 != NULL)
        {
            char* ptr = (char*)realloc(mem->memory, mem->size + strlen(item_1->valuestring) + 1);
            if (ptr == NULL) {
                printf("realloc() failed\n");
                cJSON_Delete(response);
                ClearJBuf(json_s, result);
                return 0;
            }
            mem->memory = ptr;
            memcpy(&(mem->memory[mem->size]), item_1->valuestring, strlen(item_1->valuestring));
            mem->size += strlen(item_1->valuestring);
            mem->memory[mem->size] = 0;
            int len = MultiByteToWideChar(CP_UTF8, 0, item_1->valuestring, -1, NULL, 0);
            wchar_t* transform = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, item_1->valuestring, -1, transform, len);
            len = WideCharToMultiByte(CP_ACP, 0, transform, -1, NULL, 0, NULL, NULL);
            char* show = new char[len];
            WideCharToMultiByte(CP_ACP, 0, transform, -1, show, len, NULL, NULL);
            printf("%s", show);
            delete[] transform;
            delete[] show;
        }
        cJSON_Delete(response);
    }
    ClearJBuf(json_s, result);

    return size * nmemb;
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
        headers = curl_slist_append(headers, "Authorization: Bearer $CHATGPT_API_KEY$");    // 这里替换为自己的私钥
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
            printf("\n");
            //printf("HTTP response code: %ld\n", http_code);
            //printf("chunk memory:%s\n", chunk.memory);
            cJSON* json = cJSON_CreateObject();
            cJSON_AddStringToObject(json, "role", "assistant");
            cJSON_AddStringToObject(json, "content", chunk.memory);
            result = cJSON_Print(json);
            cJSON_Delete(json);
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
    cJSON_AddTrueToObject(json, "stream");
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
