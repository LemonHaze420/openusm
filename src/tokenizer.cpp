#include "tokenizer.h"

#include "common.h"
#include "log.h"

#include <cassert>
#include <cctype>
#include <cstring>
#include <cstdint>

tokenizer::tokenizer(bool a2)
{
    this->field_C.clear();
    this->field_8 = a2;
    this->field_4 = 0;
    this->field_0 = nullptr;
    this->field_9 = 0;
    this->field_A = 0;
    this->field_B = 0;
    this->field_1C = this->field_C.begin();
}

tokenizer::~tokenizer()
{
    clear();
}

char *tokenizer::get_token()
{
    //sp_log("tokenizer::get_token: start");

    if (this->field_0 == nullptr) {
        return nullptr;
    }

    auto end = this->field_C.end();
    if (this->field_1C == (end)) {
        return nullptr;
    }

    auto *v4 = *this->field_1C;
    ++this->field_1C;

    return v4;
}

void tokenizer::clear()
{
    if (!this->field_8 && this->field_0) {
        delete[] (this->field_0);
    }

    this->field_0 = nullptr;
    this->field_4 = 0;

    this->field_C.clear();

    this->field_1C = this->field_C.begin();
}

void tokenizer::setup_current_iterator()
{
    if (this->field_0 != nullptr) {
        this->field_1C = this->field_C.begin();
    }
}

int tokenizer::parse(char *a2, const char *a3)
{
    if (this->field_0 != nullptr) {
        this->clear();
    }

    this->field_4 = strlen(a2) + 1;
    this->field_0 = a2;
    auto v10 = strlen(a2);
    if (v10 <= 0) {
        return 0;
    }

    if (!this->field_8) {
        this->field_0 = new char[v10 + 1];
        strcpy(this->field_0, a2);
    }

    auto v9 = strlen(a3);
    auto *v8 = this->field_0;
    bool v5 = false;
    auto v7 = 0;
    uint32_t i;
    while (this->field_0[v7] || v5) {
        for (i = 0; i < v9 && this->field_0[v7] != a3[i]; ++i) {
            ;
        }

        auto v4 = this->field_0[v7];
        if (i == v9 && this->field_0[v7]) {
            v5 = true;
            if (this->field_9 && (this->field_0[v7] == '\\' || this->field_0[v7] == '/')) {
                this->field_0[v7] = this->field_9;
            } else if (this->field_B) {
                this->field_0[v7] = toupper(this->field_0[v7]);
            } else if (this->field_A) {
                this->field_0[v7] = tolower(this->field_0[v7]);
            }
        } else {
            v5 = false;
            this->field_0[v7] = '\0';
            if (strlen(v8)) {
                this->field_C.push_back(v8);
            }

            v8 = &this->field_0[v7 + 1];
        }

        if (v4) {
            ++v7;
        }
    }

    this->setup_current_iterator();
    return this->field_C.size();
}

void tokenizer_unit_test()
{
    char a2[24];
    strcpy(a2, "Your mom loves death.");
    sp_log("Testing tokenizer");
    tokenizer v3{false};

    v3.parse(a2, " ");

    sp_log("Testing token");
    auto *tokey = v3.get_token();
    assert(strcmp(tokey, "Your") == 0);

    sp_log("Testing token");
    tokey = v3.get_token();
    assert(strcmp(tokey, "mom") == 0);

    sp_log("Testing token");
    tokey = v3.get_token();
    assert(strcmp(tokey, "loves") == 0);

    sp_log("Testing token");
    tokey = v3.get_token();
    assert(strcmp(tokey, "death.") == 0);

#if 1
    auto v0 = strlen(a2);
    auto *v2 = new char[v0 + 1];
    strcpy(v2, a2);
    tokenizer v1{true};

    v1.parse(v2, " ");
    sp_log("Testing in place token");
    tokey = v1.get_token();

    assert(strcmp(tokey, "Your") == 0);

    sp_log("Testing in place token");
    tokey = v1.get_token();
    assert(strcmp(tokey, "mom") == 0);
    sp_log("Testing in place token");
    tokey = v1.get_token();
    assert(strcmp(tokey, "loves") == 0);
    sp_log("Testing in place token");
    tokey = v1.get_token();
    assert(strcmp(tokey, "death.") == 0);

    delete[] v2;
#endif
}
