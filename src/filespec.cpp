#include "filespec.h"

#include "func_wrapper.h"
#include "log.h"
#include "utility.h"
#include "variables.h"

mString filespec::fullname() const {
    mString a2 = this->m_dir + this->m_name + this->m_ext;
    return a2;
}

mString filespec::name() const {
    mString res = this->m_name + this->m_ext;
    return res;
}

filespec::filespec(const mString &a2, const mString &a3, const mString &a4)
    : m_dir(a2), m_name(a3), m_ext(a4) {
}

filespec::filespec(const mString &a2) : m_dir(), m_name(), m_ext() {
    this->extract(a2);
}

void filespec::extract(const mString &a2) {
    int v4 = a2.rfind('\\', -1);
    int v5 = 0;
    if (v4 == -1) {
        m_dir = "";
    } else {
        v5 = v4 + 1;
        mString v6 = a2.substr(0, v4 + 1);

        m_dir = v6;
    }

    int v7 = a2.rfind('.', -1);
    int v9;

    if ( (v7 == -1) || ((v9 = a2.rfind('.', v7 - 1), v9 != -1) && (v9 + 1 == v7)) ) {
        m_name = a2.substr(v5, a2.size() - v5 + 1);

        m_ext = "";
    } else {
        m_name = a2.substr(v5, v7 - v5);

        m_ext = a2.substr(v7, a2.size() - v7);
    }
}

void filespec_patch() {
    FUNC_ADDRESS(address, &filespec::extract);
    REDIRECT(0x0055CBC5, address);
}
