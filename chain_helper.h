#pragma once

#include <TChain.h>
#include <TBranch.h>
#include <vector>
#include <string>
#include <is_cont.h>

template <typename... branch_type>
class root_chain
{
private:
    TChain *chain{};
    std::tuple<decltype(new branch_type)...> data;
    std::array<TBranch *, sizeof...(branch_type)> b_add;
    const char *tree_name;

public:
    template <class T, T I, T... J>
    void do_setaddress(std::integer_sequence<T, I, J...>, std::array<const char *, sizeof...(branch_type)> names)
    {
        do_setaddress(std::integer_sequence<std::size_t, I>{}, names);
        do_setaddress(std::integer_sequence<std::size_t, J...>{}, names);
    }
    template <class T, T I>
    void do_setaddress(std::integer_sequence<T, I>, std::array<const char *, sizeof...(branch_type)> names)
    {
        int status;
        b_add[I] = chain->GetBranch(std::get<I>(names));
        assert(b_add[I]);
        if (!is_cont<typename std::tuple_element<I, std::tuple<branch_type...>>::type>::value)
        {
            // normal case, assign space and SetBranchAddress, the ugly expression
            // is for properly dealing with arrays, the assigned space is then freed
            // by me (not ROOT)
            std::get<I>(data) = new typename std::tuple_element<I, std::tuple<branch_type...>>::type;
            chain->SetBranchAddress(std::get<I>(names), std::get<I>(data), &b_add[I]);
            // folling method is broken, don't know why:
            // b_add[I]->SetAddress(std::get<I>(data));
        }
        else
        {
            // ok, seems for STL containers, CERN ROOT will only accept T** pointer, and
            // assign space on its own, free on its own
            std::get<I>(data) = nullptr;
            chain->SetBranchAddress(std::get<I>(names), &std::get<I>(data), &b_add[I]);
        }
    }
    root_chain(std::vector<std::string> file_list, const char *tree_name, std::array<const char *, sizeof...(branch_type)> names) : tree_name(tree_name)
    {
        chain = new TChain(tree_name);
        for (const auto &i : file_list)
        {
            chain->Add(i.c_str());
        }
        do_setaddress(std::make_integer_sequence<std::size_t, sizeof...(branch_type)>{}, names);
    }
    template <class T, T I, T... J>
    void do_delete(std::integer_sequence<T, I, J...>)
    {
        do_delete(std::integer_sequence<std::size_t, I>{});
        do_delete(std::integer_sequence<std::size_t, J...>{});
    }
    template <class T, T I>
    void do_delete(std::integer_sequence<T, I>)
    {
        if (!is_cont<typename std::tuple_element<I, std::tuple<branch_type...>>::type>::value)
            delete[] std::get<I>(data);
    }

    ~root_chain()
    {
        chain->ResetBranchAddresses();
        // for (auto i : b_add)
        //     delete i;
        do_delete(std::make_integer_sequence<std::size_t, sizeof...(branch_type)>{});
    }

    std::tuple<decltype(new branch_type)...> &get(std::size_t id)
    {
        auto curr = chain->LoadTree(id);
        if (curr < 0)
        {
            throw;
        }
        for (auto &i : b_add)
            i->GetEntry(curr);
        return data;
    }
    auto get_up()
    {
        return chain->GetEntries();
    }
    // used by range-based-for
    class iter
    {
    private:
        long num;
        root_chain *se;

    public:
        iter(int m_num, root_chain *root_chain_self)
        {
            se = root_chain_self;
            num = m_num;
        }
        iter &operator++()
        {
            num++;
            return *this;
        }
        bool operator!=(const iter &other) const
        {
            return num != other.num;
        }
        auto &operator*()
        {
            return se->get(num);
        }
    };
    auto begin()
    {
        return iter(0, this);
    }
    auto end()
    {
        return iter(get_up(), this);
    }
};
