//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include "Link.h"
#include "Link_type.h"
#include <memory>

class Block;

// neighbor-links map for blocks (static in Building)
using blink_t = Link<bid_t>;
using bptr_t = std::shared_ptr<Block>;
using bmap_t = map<bid_t, bptr_t>;

class Block {
    public:
        Block(){
        }
        Block(bid_t _self, blink_t::mmap_it it_first, const blink_t::mmap_it it_last):
            self(_self), link(it_first, it_last)
        {}
        //
        const bid_t self;
        // map of all blocks
        static bmap_t blocks;
        // public helper functions
        size_t size() {
            return house_id.size();
        }
        bool isEmpty() {
            return size() == 0;
        }
        hid_t GetRandomHouse() {
            size_t draw = RandomIndexFlat(house_id.size());
            return house_id[draw];
        }
        hid_t LRMigration() {
            // draw neighbor block name
            auto dest_block = link.GetRandomNeighbor();
            // get ptr_to_block from block name 
            // then draw house name from neighbor block
            hid_t house_dest = blocks.at(dest_block)->GetRandomHouse();
            return house_dest;
        }
        void add_house(hid_t id){
            house_id.push_back(id);
        }
    private:
        Link<bid_t> link;
        // houses in this block
        vector<size_t> house_id;
};
