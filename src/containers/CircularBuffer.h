/*
  ==============================================================================

    circ_buf.h
    Created: 27 Oct 2023 11:11:15pm
    Author:  Kozaróczy Csaba

  ==============================================================================
*/

#pragma once

template <class T>
class CircularBuffer {
public:
    //==================================================================
    CircularBuffer(int bufferSize = 0)
        : size(bufferSize)
    {
        if (size < 0) throw("negative size");

        base = new T[size];
        cur = 0;

        for (int i = 0; i < size; i++)
            base[i] = 0;
    }
    ~CircularBuffer() {
        delete[] base;
    }
    //==================================================================
    int getSize() const
    {
        return size;
    }
    T push(T _new) {

        if (size == 0) return _new;

        T last = base[cur];
        base[cur++] = _new;
        if (cur >= size) cur -= size;
        return last;
    }
    T& getCur()
    {
        return base[cur];
    }
    T& latest()
    {
        if (cur == 0) return base[size - 1];
        else return base[cur - 1];
    }
    void rotate(int i = 1)
    {
        (cur += i) %= size;
    }
    void resize(int _size)
    {
        if (size == _size) return;

        flatten();
        T* new_buf = new T[_size];

        for (int i = 1; i <= _size; i++) 
        {
            if (i <= size)
                new_buf[_size - i] = base[size - i];
            else
                new_buf[_size - i] = 0;
        }

        delete[] base;
        base = new_buf;

        size = _size;
    }
    void flatten()
    {
        T* flat = new T[size];
        int i = 0;

        for (i; i < size - cur; i++)        // whatever was left in the end of the buffer
            flat[i] = base[cur + i];
        for (i; i < size; i++)                  // most recent data
            flat[i] = base[i - size + cur];

        cur = 0;
        delete[] base;
        base = flat;
    }
    //==================================================================
private:
    int size;
    T*  base;
    // always pointing at the oldest element, about to be overwritten
    int cur;
};