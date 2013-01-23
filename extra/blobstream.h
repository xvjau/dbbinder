/*
    Copyright 2013 Gianni Rossi

    This file is part of DBBinder++.

    DBBinder++ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DBBinder++ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DBBinder++.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DBBINDERBLOBSTREAM_H
#define DBBINDERBLOBSTREAM_H

#include <vector>
#include <istream>
#include <cstring>

namespace DBBinder
{

/**
 * Helper class for generated code which uses shared_ptr<vector<char>> for blob fields
 */
class BlobStream
{
private:
    typedef char CharT;
    typedef std::char_traits<CharT> TraitsT;
    typedef std::vector<CharT> vector_t;
    typedef std::shared_ptr<vector_t> vector_ref_t;

    class VectorWrapper : public std::basic_streambuf<CharT, TraitsT> {
    private:
        vector_ref_t _vector;
    public:
        VectorWrapper(vector_ref_t vector):
            _vector(vector)
        {
            vector_t *v = _vector.get();
            setg(v->data(), v->data(), v->data() + v->size());
            setp(v->data(), v->data() + v->size());
        }
    protected:
        virtual std::streamsize xsputn(const char * s, std::streamsize n) override
        {
            vector_t *v = _vector.get();

            if (v->empty())
            {
                v->resize(n);
                memcpy(v->data(), s, n);
            }
            else
            {
                auto size = v->size();
                v->resize(size + n);
                memcpy(v->data() + size, s, n);
            }

            setp(v->data(), v->data() + v->size());
        }

        virtual int overflow( int c ) override
        {
            vector_t *v = _vector.get();

            auto size = v->size();
            v->resize(size + 1);
            v->data()[size] = c;

            return 1;
        }
    };

    VectorWrapper _wrapper;

    std::istream _istream;
    std::ostream _ostream;

public:
    BlobStream(vector_ref_t vector):
        _wrapper(vector),
        _istream(&_wrapper),
        _ostream(&_wrapper)
    {
    }

    std::istream* istream()
    {
        return &_istream;
    }

    std::ostream* ostream()
    {
        return &_ostream;
    }
};

}

#endif // DBBINDERBLOBSTREAM_H
