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
 * @class BlobStream
 * Helper class for Blob field.
 *
 * This class helps create a std::stream interface to a
 * shared_ptr<vector<char>> object which might be the
 * C++ type used for blob/TEXT fields.
 *
 * IStream example:
 *
 *    auto blob = std::make_shared< std::vector<char>>();
 *    DBBinder::BlobStream stream(blob);
 *    pb->serializeToOstream(stream.ostream());
 *    ins.insert(1, blob);
 *
 * OStream example::
 *
 *    DBBinder::BlobStream str(j->getpbData());
 *    pb->parseFromIstream(str.istream());
 */
class BlobStream
{
private:
    typedef char char_t;
    typedef std::char_traits<char_t> traits_t;
    typedef std::vector<char_t> vector_t;
    typedef std::shared_ptr<vector_t> vector_ref_t;

    class VectorWrapper : public std::basic_streambuf<char_t, traits_t> {
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
                setp(v->data(), v->data() + v->size());
                memcpy(pptr(), s, n);
            }
            else
            {
                auto size = v->size();
                auto remain = epptr() - pptr();

                if (remain < n)
                {
                    auto delta = n - remain;
                    v->resize(size + delta);
                    setp(v->data(), v->data() + v->size());
                }
                memcpy(pptr(), s, n);
            }
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
