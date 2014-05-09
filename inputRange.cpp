#include <iostream>
#include <utility>

int numeric_range() { static int idx = std::ios_base::xalloc(); return idx; }

void erase_range(std::ios_base::event evt, std::ios_base& str, int index)
{
    if (evt == std::ios_base::erase_event)
    {
        void*& p = str.pword(index);
        delete static_cast<std::pair<int, int>*>(p);
        p = nullptr;
    }
}

class num_get : public std::num_get<char>
{
public:
    iter_type do_get( iter_type in, iter_type end, std::ios_base& str, 
                      std::ios_base::iostate& err, long& v ) const
    {
        long temp;
        in = std::num_get<char>::do_get(in, end, str, err, temp);

        if (in != end)
            return in;

        if (void*& p = str.pword(numeric_range()))
        {
            auto values = *static_cast<std::pair<int, int>*>(p);
            if (!(values.first <= temp && temp <= values.second))
            {
                err |= std::ios_base::failbit;
            } else
                v = temp;
        }
        return in;
    }
};

void deallocate(std::ios& str, int idx)
{
    erase_range(std::ios_base::erase_event, str, idx);
}

template<int I, int J>
std::istream& set_range(std::istream& is)
{
    deallocate(is, numeric_range());

    if (is)
    {
        is.pword(input_range()) = new std::pair<int, int>(I, J);

        if (!dynamic_cast<const num_get*>(
                &std::use_facet<std::num_get<char>>(is.getloc())))
        {
            is.imbue(std::locale(is.getloc(), new num_get));
            is.register_callback(&erase_range, numeric_range());
        }
    }
    return is;
}

std::istream& unset_range(std::istream& is)
{
    deallocate(is, numeric_range());
    return is;
}