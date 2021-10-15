/**
 * 
 * Options:
 *  USE_HEAD_TAIL
 * 
*/

#include <iostream>
#include <string.h>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <memory>
#include <limits>
#include "basic.hpp"



#ifdef USE_HEAD_TAIL
class Slicing_Single{
public:
    int position; 
    bool left;
    Slicing_Single(const int& _position) : position(_position), left(false){}
    Slicing_Single(int _position,bool _left) : position(_position),left(_left) {}

};


class Slicing_rr_empty {
public:
};


Slicing_Single operator == (int idx,Slicing_rr_empty b){
    return Slicing_Single(idx,true);
}

    #define head == Slicing_rr_empty()
    #define tail (Slicing_Single)
#endif


template<class value_type>
class Array;

class To_Range{
public:
    size_t start;
    size_t end;
    To_Range(size_t _start,size_t _end) :
    start(_start), end(_end) {}

    operator Array<int>();

};

class Slicing_To_End{
public:
    int end;
    Slicing_To_End(const int& init) : end(init) {}
};

To_Range operator == (const int& start,const Slicing_To_End& end) {
    return To_Range(start,end.end);
}

#define to == (Slicing_To_End)

class SubscriptStep {
public:
    int value;
    SubscriptStep() : value(1) {}
    SubscriptStep(const int& init) : value(init) {}
};

#define _ SubscriptStep()
#define __ (SubscriptStep)


class ShapeElem{
public:
    ShapeElem* next;
    int len;

    ShapeElem(int _len,ShapeElem* _next): next(_next),len(_len){}

    void to_array(size_t** shape_array,size_t* shape_len,size_t* data_len){
        *shape_len = this->depth();
        *shape_array = new size_t[*shape_len];
        *data_len = 1;
        this->assign(*shape_array,data_len);
    }

    void assign(size_t* shape_array,size_t* data_len){
        *data_len = (*data_len) * len;
        *shape_array = len;
        shape_array = shape_array + 1;
        if(next != nullptr){
            next->assign(shape_array,data_len);
        }
    }


    int depth(){
        if(next == nullptr){
            return 1;
        }else{
            return next->depth() +1;
        }

    }

    ~ShapeElem(){
        if(next != nullptr){
            delete next;
        }
    }
};

template<class value_type>
class ArrayInit{
public:
    void* data = nullptr;
    size_t len;
    bool is_final;

    ArrayInit(std::initializer_list<value_type> init) : data((void*)init.begin()), len(init.size()),is_final(true){}

    ArrayInit(std::initializer_list<ArrayInit<value_type>> init): data((void*)init.begin()), len(init.size()),is_final(false){}

    ShapeElem* shape(){
        ShapeElem* out;
        if(is_final){
            out = new ShapeElem(len,nullptr);
        }else{
            ArrayInit<value_type>* first = (ArrayInit<value_type>*)data;
            out = new ShapeElem(len,first->shape());
        }
        return out;
    }
    /*
    void assign(Object** pointer){
        if(is_final){
            memcpy(*pointer,data,sizeof(value_type) * len);
            (*pointer) = (*pointer) + len;
            Object* tarray = (Object*)data;
            for(size_t k = 0; k < len;k++){
                tarray[k].data = nullptr;
            }
        }else{
            ArrayRef<value_type>* data_array = (ArrayRef<value_type>*)data;
            for(int k = 0;k < len;k++){
                data_array[k].assign(pointer);
            }
        }
    }*/

    void assign(value_type** pointer){
        if(is_final){
            for(size_t k = 0; k < len;k ++ ){
                (*pointer)[k] =  ( ((value_type*)data)[k]);
            }
            //memcpy(*pointer,data,sizeof(value_type) * len);
            (*pointer) = (*pointer) + len;
        }else{
            ArrayInit<value_type>* data_array = (ArrayInit<value_type>*)data;
            for(int k = 0;k < len;k++){
                data_array[k].assign(pointer);
            }
        }
    }
};







template<class value_type>
class Array;


template<class value_type>
class Subscript{
public:
    Subscript<value_type>* first;
    Subscript<value_type>* next = nullptr;
    Array<value_type>* array;
    size_t* idx;
    size_t idx_len;
    int depth = 0;

    Subscript(Array<value_type>* _array,Subscript<value_type>* _first,int _depth,int _idx) : array(_array), first(_first), depth(_depth){
        idx_len = 1;
        idx = new size_t[idx_len];
        if(_idx < 0){
            idx[0] = array->arrayRef->shape_array[depth] + _idx;
        }else{
            idx[0] = _idx;
        }
    }

    Subscript(Array<value_type>* _array,Subscript<value_type>* _first,int _depth,bool* _idx) : array(_array), first(_first), depth(_depth){
        int idx_array_len = _array->arrayRef->shape_array[depth];
        idx_len = 0;
        for(int k = 0;k < idx_array_len;k++){
            idx_len += _idx[k];
        }

        idx = new size_t[idx_len];

        int i=0;
        for(int k = 0;k < idx_array_len;k++){
            if(_idx[k]){
                idx[i] = k;
                i++;
            }
        }
    }

    //***************slicing begin****************
    Subscript(Array<value_type>* _array,Subscript<value_type>* _first,int _depth,To_Range seperator) : array(_array), first(_first), depth(_depth){
        size_t full_size = _array->arrayRef->shape_array[depth];
        int start = seperator.start;
        int end = seperator.end;
        if(start < 0){
            start = full_size + start;
        }
        if(end < 0){
            end = full_size + end;
        }
        
        idx_len = end - start;
        idx = new size_t[idx_len];
        size_t i = 0;
        for(size_t k = start;k < end;k++){
           idx[i] = k;
           i++;
        }
    }

#ifdef USE_HEAD_TAIL
    Subscript(Array<value_type>* _array,Subscript<value_type>* _first,int _depth,Slicing_Single seperator) : array(_array), first(_first), depth(_depth){
        int position = 0;
        size_t full_size = _array->arrayRef->shape_array[depth];
            
        if(seperator.position < 0){
            position = full_size + seperator.position;
        }else{
            position = seperator.position;
        }
        bool left = seperator.left;
        if(left){

            idx_len = full_size - position;
            idx = new size_t[idx_len];

            for(size_t k = 0; k < idx_len;k++){
                idx[k] = k + position;
            }
        }else{
            idx_len = position;
            idx = new size_t[position];
            
            for(size_t k = 0; k < position;k++){
                idx[k] = k;
            }
        }
    }
#endif

    Subscript(Array<value_type>* _array,Subscript<value_type>* _first,int _depth,SubscriptStep _direction) : array(_array), first(_first), depth(_depth){
        int direction = _direction.value;
        if(direction < 0){
            int step_size = -direction;
            size_t full_size = _array->arrayRef->shape_array[depth];
            idx_len = full_size / step_size;
            idx = new size_t[idx_len];
            for(int k = 0;k < idx_len;k++){
                idx[k] = full_size - k * step_size - 1;
            }
        }else{
            size_t full_size = _array->arrayRef->shape_array[depth];
            idx_len = full_size / direction;
            idx = new size_t[idx_len];
            for(int k = 0;k < idx_len;k++){
                idx[k] = k * direction;
            }
        }
        
    }

    Subscript(Array<value_type>* _array,Subscript<value_type>* _first,int _depth,std::initializer_list<bool> _idx) : array(_array), first(_first), depth(_depth){
        int idx_array_len = _idx.size();
        idx_len = 0;
        for(auto elem : _idx){
            idx_len += elem;
        }

        idx = new size_t[idx_len];

        int i=0;
        for(int k = 0;k < idx_array_len;k++){
            if(((bool*)_idx.begin())[k]){
                idx[i] = k;
                i++;
            }
        }
    }

    Subscript(Array<value_type>* _array,Subscript<value_type>* _first,int _depth,std::initializer_list<int> _idx) : array(_array), first(_first),depth(_depth){
        idx_len = _idx.size();
        idx = new size_t[idx_len];
        for(int k=0; k < idx_len;k++){
            int tmp = ( (int*)_idx.begin() )[k];
            if(tmp < 0){
                idx[k] = array->arrayRef->shape_array[depth] + tmp;
            }else{
                idx[k] = tmp;
            }
        }
        //memcpy(idx,(int*)_idx.begin(),sizeof(size_t) * idx_len);
    }

    void assign_data(value_type** pointer,size_t subscript_pointer){
        //std::cout << "idx";
        for(int k = 0; k < idx_len;k++){
            //std::cout << idx[k] << " ";
            if(next != nullptr){
                next->assign_data(pointer,subscript_pointer + idx[k] * array->get_subscript_mul(depth));
            }else{
                memcpy((*pointer),&(array->arrayRef->data_array[subscript_pointer + idx[k] * (array->get_subscript_mul(depth))]),(array->get_subscript_mul(depth))*sizeof(value_type));
                (*pointer) = (*pointer) + (array->get_subscript_mul(depth));
            }
        }
    }
    Array<value_type>& operator= (Subscript<value_type>& input){
        int final_idx = 0;
        int k = 0;

        for(Subscript<value_type>* runner = first; runner != nullptr; runner = runner->next){
            final_idx += (runner->idx[0])* array->get_subscript_mul(k);
            k++;
        }
        array->arrayRef->data_array[final_idx] = input;
        return *array;
    }

    Array<value_type>& operator= (value_type input){
        int final_idx = 0;
        int k = 0;

        for(Subscript<value_type>* runner = first; runner != nullptr; runner = runner->next){
            final_idx += (runner->idx[0])* array->get_subscript_mul(k);
            k++;
        }
        /*if(first->idx[0] == 0&& first->next->idx[0] == 0){// && first->next->idx == 0
            std::cout << "goedit\n";
        }*/
        //std::cout << "idx == "<< first->idx[0] <<"\n";
        array->arrayRef->data_array[final_idx] = input;
        return *array;
    }

    //*********fancy way for Object == value_type***
    //template<class other_type,typename = typename std::enable_if<std::is_same<Object,value_type>::value >::type>
    /*operator double(){
        value_type out = this->operator value_type();
        double tout = out;
        ((Object*)(&out))->data = nullptr;
        std::cout << "type "<< out.type << double(out) << "\n";
        return ;
    }*/

    /*template<class other_type,typename = typename std::enable_if<std::is_same<Object,value_type>::value >::type>
    operator Array<other_type>(){
        Array<value_type> out = this->operator Array<value_type>();
        return out.operator Array<other_type>();
    }*/
    //*********fancy way for Object == value_type***

    operator value_type(){
        int final_idx = 0;
        int k = 0;
        for(Subscript<value_type>* runner = first; runner != nullptr; runner = runner->next){
            final_idx += (runner->idx[0])* array->get_subscript_mul(k);
            k++;
        }
        //std::cout << "idx = " <<  first->idx[0] << " " <<  first->idx[0] << "\n";
        return array->arrayRef->data_array[final_idx];
    }
    /*template<class other_type>
    operator other_type(){

    }*/

    operator Array<value_type>(){
        size_t num_idx_len_1 = 0;
        for(Subscript<value_type>* runner = first; runner != nullptr; runner = runner->next){
            if(runner->idx_len == 1){
                num_idx_len_1++;
            }
        }
        size_t* shape_array = nullptr;
        size_t shape_len = array->arrayRef->shape_len  - num_idx_len_1;
        if(shape_len == 0){
            shape_array = new size_t[1];
            shape_len = 1;
            shape_array[0] = 1;
        }else{
            shape_array = new size_t[shape_len];
        }

        size_t data_len = 1;
        int i = 0;
        for(Subscript<value_type>* runner = first; runner != nullptr; runner = runner->next){
            if(runner->idx_len != 1){
                shape_array[i] = runner->idx_len;
                data_len = (runner->idx_len) * data_len;
                i++;
            }
        }
        for(int k = depth + 1;k < array->arrayRef->shape_len;k++){
            shape_array[i] = array->arrayRef->shape_array[k];
            data_len = (array->arrayRef->shape_array[k]) * data_len;
            i++;
        }

        value_type* data_array = new value_type[data_len];
        value_type* pointer = data_array;
        first->assign_data(&pointer,0);
        return Array<value_type>(shape_array,shape_len,data_array,data_len);
    }

    /*Subscript(Array<value_type>* _array,Array<int>* _idx) : array(_array) {
        //idx_len = _idx.size();
        //idx = new int[data_len];
        //memcpy(idx,(int*)_idx.begin(),sizeof(int) * idx_len);
    }*/

    template<class idx_type>
    Subscript<value_type> operator[](idx_type idx){
        auto out = Subscript<value_type>(array,first,depth+1,idx);
        next = &out;
        return out;
    }

    template<class idx_type>
    Subscript<value_type> operator[](std::initializer_list<idx_type> idx){
        auto out = Subscript<value_type>(array,first,depth+1,idx);
        next = &out;
        return out;
    }

    ~Subscript(){
        if(idx != nullptr){
            delete[] idx;
        }
    }

};

template<class value_type>
class ArrayRef{
public:
    size_t* shape_array = nullptr;
    size_t shape_len;

    value_type* data_array = nullptr;
    size_t data_len;

    ~ArrayRef(){
        if(shape_array != nullptr){
            delete[] shape_array;
        }
        if(data_array != nullptr){
            delete[] data_array;
        }
    }

};



template<class value_type>
class Array{//Object
public:

    /**
     * @param string output of operator std::string()
     * @param depth current depth - shape
     * @param subscript_pointer current position in the array
    */
    void assign_to_string(std::string& out,size_t depth,size_t subscript_pointer){
        out += "{";
        if(depth == arrayRef->shape_len - 1){
            for(size_t k = 0;k < arrayRef->shape_array[depth] -1;k++){
                auto tmp =  arrayRef->data_array[subscript_pointer + k * this->get_subscript_mul(depth)];
                out += str(tmp);
                out += ",";
            }
            auto tmp =  arrayRef->data_array[subscript_pointer + (arrayRef->shape_array[depth] -1)* this->get_subscript_mul(depth)];
            out += str(tmp);
        }else{
            for(size_t k = 0;k < arrayRef->shape_array[depth] - 1;k++){
                assign_to_string(out,depth + 1,subscript_pointer + k * this->get_subscript_mul(depth));
                out += ",";
            }
            assign_to_string(out,depth + 1,subscript_pointer + ( arrayRef->shape_array[depth] - 1) * this->get_subscript_mul(depth));         
        }
        out += "}";
    }

    operator std::string(){
        std::string out = "Array";
        assign_to_string(out,0,0);
        return out;
    }

    ~Array(){

    }
    //https://www.acodersjourney.com/top-10-dumb-mistakes-avoid-c-11-smart-pointers/
    //shared_ptr ppAircraft(new Aircraft[3], [](Aircraft* p) {delete[] p; });
    //std::shared_ptr<Foo>

    std::shared_ptr<ArrayRef<value_type>> arrayRef;

    size_t shape(int idx) const {
        return arrayRef->shape_array[idx];
    }

    template<class T>
    Array<value_type> operator - (Array<T>& other) {
        Array<value_type> out = Array<value_type>(this);
        for(int k =0; k < arrayRef->data_len; k++){
            out.arrayRef->data_array[k] = arrayRef->data_array[k] - other.arrayRef->data_array[k];
        }
        return out;
    }

    template<class T>
    Array<value_type> operator + (Array<T>& other) {
        Array<value_type> out = Array<value_type>(this);
        for(int k =0; k < arrayRef->data_len; k++){
            out.arrayRef->data_array[k] = arrayRef->data_array[k] + other.arrayRef->data_array[k];
        }
        return out;
    }

    template<class T>
    Array<value_type> operator * (Array<T>& other) {
    Array<value_type> out = Array<value_type>(this);
        for(int k =0; k < arrayRef->data_len; k++){
            out.arrayRef->data_array[k] = arrayRef->data_array[k] * other.arrayRef->data_array[k];
        }
        return out;
    }

    template<class T>
    Array<value_type> operator & (Array<T>& other) {
        //matmul
    }

    template<class... Args>
    Array& _reshape(Args const &... args){
        /*reshape array inplace*/
        std::initializer_list<int> list = {args...};
        arrayRef->shape_len = list.size();
        delete[] arrayRef->shape_array;
        arrayRef->shape_array = new size_t[arrayRef->shape_len];
        int k = 0;
        for(auto elem : list){
            arrayRef->shape_array[k] = elem;
            k++;
        }
        return *this;
    }

    template<class... Args>
    Array reshape(Args const &... args){
        /*create new array and reshape it*/
        std::initializer_list<int> list = {args...};

        Array out = Array();
        out.arrayRef->data_len = arrayRef->data_len;
        out.arrayRef->data_array = new value_type[arrayRef->data_len];
        for(int k = 0;k < arrayRef->data_len;k++){
            out.arrayRef->data_array[k] = arrayRef->data_array[k];
        }
        out.arrayRef->shape_len = list.size();
        out.arrayRef->shape_array = new size_t[out.arrayRef->shape_len];
        
        int k = 0;
        for(auto elem : list){
            out.arrayRef->shape_array[k] = elem;
            k++;
        }
        return out;
    }



    operator Array<value_type>(){
        return Array<value_type>(this);
    }

    template<class new_type>
    Array<new_type> astype(){
        return Array<new_type>();
    }

    Array<bool> all(int dim = -1){
        if(!std::is_same<value_type,bool>::value){
            throw std::runtime_error("this function is not defined for none bool arrays");
        }
        size_t edata_len;
        if(dim == -1){
            edata_len = 1;
        }else{
            edata_len = arrayRef->shape_array[dim];
        }
        size_t eshape_len = 1;
        size_t* eshape_array = new size_t[1];
        bool* edata_array = new bool[1];

        eshape_array[0] = edata_len;
        for(size_t k = 0;k < edata_len;k++){
            bool any_false = false;
            for(size_t j = 0;j < get_subscript_mul(dim);j++){
                if(!arrayRef->data_array[get_subscript_mul(dim) * k + j]){
                    any_false = true;
                }
            }
            edata_array[k] = !any_false;
        }
        return Array<bool>(eshape_array,eshape_len,edata_array,edata_len);
    }

    Array<bool> any(int dim = -1){
        if(!std::is_same<value_type,bool>::value){
            throw std::runtime_error("this function is not defined for none bool arrays");
        }
        size_t edata_len;
        if(dim == -1){
            edata_len = 1;
        }else{
            edata_len = arrayRef->shape_array[dim];
        }
        size_t eshape_len = 1;
        size_t* eshape_array = new size_t[1];
        bool* edata_array = new bool[1];

        eshape_array[0] = edata_len;
        for(size_t k = 0;k < edata_len;k++){
            bool any_true = false;
            for(size_t j = 0;j < get_subscript_mul(dim);j++){
                if(arrayRef->data_array[get_subscript_mul(dim) * k + j]){
                    any_true = true;
                }
            }
            edata_array[k] = any_true;
        }
        return Array<bool>(eshape_array,eshape_len,edata_array,edata_len);
    }

    template<class A>
    bool same_shape(const Array<A>& other){
        if(arrayRef->shape_len != other.arrayRef->shape_len){
            return false;
        }
        for(int k = 0;k < arrayRef->shape_len;k++){
            if(arrayRef->shape_array[k] != other.arrayRef->shape_array[k]){return false;}
        }
        return true;
    }

    Array<bool> compare(const Array<value_type>& other,bool (*comp_func)(value_type,value_type)){
        if(!same_shape(other)){
            throw std::runtime_error("can't compare two arrays with different shapes");
        }

        size_t eshape_len = arrayRef->shape_len;
        size_t edata_len = arrayRef->data_len;

        size_t* eshape_array = new size_t[eshape_len];
        bool* edata_array = new bool[edata_len];

        memcpy(eshape_array,arrayRef->shape_array,arrayRef->shape_len* sizeof(size_t));

        for(size_t k = 0;k < arrayRef->data_len;k++){
            edata_array[k] = ((*comp_func)( arrayRef->data_array[k] , other.arrayRef->data_array[k] ));
        }
        return Array<bool>(eshape_array,eshape_len,edata_array,edata_len);
    }

    Array<bool> operator ==(const Array<value_type>& other){
        return compare(other,[](int a,int b){return a == b;});
    }

    Array<bool> operator >=(const Array<value_type>& other){
        return compare(other,[](int a,int b){return a >= b;});
    }

    Array<bool> operator <=(const Array<value_type>& other){
        return compare(other,[](int a,int b){return a <= b;});
    }

    Array<bool> operator >(const Array<value_type>& other){
        return compare(other,[](int a,int b){return a > b;});
    }

    Array<bool> operator <(const Array<value_type>& other){
        return compare(other,[](int a,int b){return a < b;});
    }


    //******** constructors and co*******
    inline
    size_t get_subscript_mul(int idx) const {
        size_t out = 1;
        for(int k = idx+1; k < arrayRef->shape_len;k++){
            out = out * arrayRef->shape_array[k];
        }
        return out;
    }

    template<class idx_type>
    Subscript<value_type> operator[](idx_type idx){
        auto out = Subscript<value_type>(this,nullptr,0,idx);
        out.first = &out;
        return out;
    }

    template<class idx_type>
    Subscript<value_type> operator[](std::initializer_list<idx_type> idx){
        auto out = Subscript<value_type>(this,nullptr,0,idx);
        out.first = &out;
        return out;
    }


    void _reverse_data(){
        //this is a in between solution
        value_type* tmp = new value_type[arrayRef->data_len];
        for(size_t k = 0;k < arrayRef->data_len;k++){
            tmp[k] = arrayRef->data_array[k];
        }
        for(size_t k = 0;k < arrayRef->data_len;k++){
            arrayRef->data_array[k] = tmp[arrayRef->data_len - k -1];
        }
        delete[] tmp;
    }

    Array copy(){
        Array out = Array();
        out.arrayRef->data_len = arrayRef->data_len;
        out.arrayRef->shape_len = arrayRef->shape_len;
        out.arrayRef->shape_array = new size_t[arrayRef->shape_len];
        out.arrayRef->data_array = new value_type[arrayRef->data_len];
        for(int k = 0;k < arrayRef->shape_len;k++){
            out.arrayRef->shape_array[k] = arrayRef->shape_array[k];
        }
        for(int k = 0;k < arrayRef->data_len;k++){
            out.arrayRef->data_array[k] = arrayRef->data_array[k];
        }
        return out;
    }

    Array<value_type>& operator =(const Array<value_type>& other){
        this->arrayRef = other.arrayRef;
        return *this;
    }

    Array(){
        arrayRef = std::shared_ptr<ArrayRef<value_type>>(new ArrayRef<value_type>());
    }

    Array(std::shared_ptr<ArrayRef<value_type>> pointer){
        arrayRef = pointer;
    }

    Array(Array<value_type>* other) {
        arrayRef = other->arrayRef;
    }

    Array(const Array<value_type>* other) {
        arrayRef = other->arrayRef;
    }

    Array(const Array<value_type>& other){
        arrayRef = other.arrayRef;
    }
    
    Array(Array<value_type>& other){
        arrayRef = other.arrayRef;
    }

    Array(size_t* _shape_array,size_t _shape_len,value_type* _data_array,size_t _data_len){
        arrayRef = std::shared_ptr<ArrayRef<value_type>>(new ArrayRef<value_type>());
        arrayRef->data_len = _data_len;
        arrayRef->shape_len = _shape_len;
        arrayRef->shape_array = _shape_array;
        arrayRef->data_array = _data_array;
    }

    Array(std::initializer_list<value_type> init) {
        arrayRef = std::shared_ptr<ArrayRef<value_type>>(new ArrayRef<value_type>());
        arrayRef->data_len = init.size();
        arrayRef->shape_len = 1;
        arrayRef->shape_array = new size_t[1];
        arrayRef->shape_array[0] = arrayRef->data_len;
        arrayRef->data_array = new value_type[arrayRef->data_len];

        for(size_t k = 0; k < init.size();k ++ ){
            arrayRef->data_array[k] =  ( (value_type*)init.begin() )[k];
        }
        //memcpy(arrayRef->data_array,(value_type*)init.begin(),sizeof(value_type) * arrayRef->data_len);
    }

    Array(std::initializer_list<ArrayInit<value_type>> init) {
        arrayRef = std::shared_ptr<ArrayRef<value_type>>(new ArrayRef<value_type>());
        ArrayInit<value_type>* init_array = (ArrayInit<value_type>*)(init.begin());
        auto sElem = ShapeElem(init.size(),init_array->shape());
        sElem.to_array(&arrayRef->shape_array,&arrayRef->shape_len,&arrayRef->data_len);
        arrayRef->data_array = new value_type[arrayRef->data_len];
        value_type* pointer = arrayRef->data_array;
        for(int k =0;k < init.size();k++){
            init_array[k].assign(&pointer);
        }
    }

    Array(size_t* _shape_array,size_t _shape_len) {
        arrayRef = std::shared_ptr<ArrayRef<value_type>>(new ArrayRef<value_type>());
        arrayRef->shape_len = _shape_len;
        arrayRef->shape_array = new size_t[arrayRef->shape_len];
        memcpy(arrayRef->shape_array,_shape_array,arrayRef->shape_len * sizeof(size_t));
        arrayRef->data_len = 1;
        for(int k = 0; k < arrayRef->shape_len;k++){
            arrayRef->data_len = arrayRef->shape_array[k] * arrayRef->data_len;
        }
        arrayRef->data_array = new value_type[arrayRef->data_len];
        for(size_t k = 0; k< arrayRef->data_len;k++){
            arrayRef->data_array[k] = 0;
        }
    }

    template<class... Args>
    Array(int arg1, Args const &... _init) {
        arrayRef = std::shared_ptr<ArrayRef<value_type>>(new ArrayRef<value_type>());
        std::initializer_list<int> init = {arg1,_init ...};
        arrayRef->shape_len = init.size();
        arrayRef->shape_array = new size_t[arrayRef->shape_len];
        int* array = (int*)init.begin();
        arrayRef->data_len = 1;
        for(int k = 0; k < arrayRef->shape_len; k++){
            arrayRef->shape_array[k] = array[k];
            arrayRef->data_len = arrayRef->data_len * array[k];
        }
        arrayRef->data_array = new value_type[arrayRef->data_len];
        for(size_t k = 0; k< arrayRef->data_len;k++){
            arrayRef->data_array[k] = 0;
        }
    }
};

template<class value_type>
Array<value_type> zeros_like(Array<value_type>& input){
    return Array<value_type>(input.arrayRef->shape_array,input.arrayRef->shape_len);
}

template<class value_type>
Array<value_type> zeros_like_(Array<value_type>& input){
    Array<value_type> out = Array<value_type>(input.arrayRef->shape_array,input.arrayRef->shape_len);
    return out;
}


//*************math stuff*****

#define ArrayOperatorOverloadInner1(operator_symbol)\
template<class value_type,class T>\
Array<value_type> operator operator_symbol (const Array<value_type>& array,const T& other) {\
    Array<value_type> out = Array<value_type>(&array);\
    for(int k =0; k < array.arrayRef->data_len; k++){\
        out.arrayRef->data_array[k] = array.arrayRef->data_array[k] operator_symbol other;\
    }\
    return out;}

#define ArrayOperatorOverloadInner2(operator_symbol)\
template<class value_type,class T>\
Array<value_type> operator operator_symbol (const T& other,const Array<value_type>& array) {\
    Array<value_type> out = Array<value_type>(&array);\
    for(int k =0; k < array.arrayRef->data_len; k++){\
        out.data_array[k] = other operator_symbol array.data_array[k];\
    }\
return out;}


ArrayOperatorOverloadInner1( / )
ArrayOperatorOverloadInner2( / )

ArrayOperatorOverloadInner1( + )
ArrayOperatorOverloadInner2( + )

ArrayOperatorOverloadInner1( - )
ArrayOperatorOverloadInner2( - )

ArrayOperatorOverloadInner1( * )
ArrayOperatorOverloadInner2( * )

template<class value_type>
Array<value_type> operator % (const Array<value_type>& a,const Array<value_type>& b){
    Array<value_type> out;
    size_t height = a.arrayRef->shape_array[0];
    size_t width = b.arrayRef->shape_array[1];
    value_type* result = new value_type[height*width];
    size_t* shape_array = new size_t[2];
    size_t shape_len = 2;
    size_t data_len = height* width;
    shape_array[0] = height;
    shape_array[1] = width;
    size_t a_width = a.arrayRef->shape_array[1];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            result[i * width + j] = 0;
            for (int p = 0; p < a_width; p++) {
                result[i * width + j] += a.arrayRef->data_array[i * a_width + p] * b.arrayRef->data_array[p*height+ j];
            }
        }
    }
    
    return Array<value_type>(shape_array,shape_len,result,data_len);
}



Array<int> arange(size_t start,size_t end,int step_size = 1){
    size_t shape_len = 1;
    size_t data_len = ((end - start) / step_size + 1);

    size_t* shape_array = new size_t[1];
    shape_array[0] = data_len;
    int* data_array = new int[data_len];
    for(size_t k = 0;k < data_len;k++){
        data_array[k] = k * step_size;
    }
    return Array<int>(shape_array,shape_len,data_array,data_len);
}

Array<int> arange(size_t end){
    size_t shape_len = 1;
    size_t data_len = end;

    size_t* shape_array = new size_t[1];
    shape_array[0] = end;
    int* data_array = new int[data_len];
    for(size_t k = 0;k < end;k++){
        data_array[k] = k;
    }
    return Array<int>(shape_array,shape_len,data_array,data_len);
}

To_Range::operator Array<int>(){
    return arange(start,end);
}


/*
template<class value_type>
Array<value_type> cat(const Array<value_type>& first){ //int dim = -1
    for(auto elem: tmp){
        elem
    }
}*/

template<class value_type>
void cat_assign_data(value_type* pointer,int depth,size_t subscript_mul_depth,Array<value_type>* array,int array_len,size_t dim_shape_len) {    
    int subscript_pointer = 0;
    for(int k = 0; k < array_len ; k++){
        for(int j = 0;j < array[k].shape(depth);j++){
            print("dim_shape_len*subscript_mul_depth",j * dim_shape_len*subscript_mul_depth + subscript_pointer);
            size_t t_data_size = array[k].get_subscript_mul(depth)*sizeof(value_type);
            value_type* t_target = &pointer[j * dim_shape_len*subscript_mul_depth + subscript_pointer];
            value_type* t_source = &(array[k].arrayRef->data_array[array[k].get_subscript_mul(depth) * j]);
            
            memcpy(t_target,t_source,t_data_size);
        }
        subscript_pointer += array[k].get_subscript_mul(depth) * array[k].shape(depth);
        print(subscript_pointer);
    }
}

template<class value_type>
Array<value_type> cat(std::initializer_list<Array<value_type>> input,int dim = 0){//int dim = INT_MIN){ //int dim = -1
    //this function isn t working yet
    if(std::numeric_limits<int>::max() == dim){
        //create mew dim;    
    }
    //all axis must be same except dim
    Array<value_type>* array = (Array<value_type>*)input.begin();
    if(dim < 0){
        dim = array->arrayRef->shape_len + dim;
    }
    if(array->arrayRef->shape_len <= dim or dim < 0){
        throw std::runtime_error("invalid dim = " + str(dim) + " shoud be less then shape_len = " + str(array->arrayRef->shape_len));
    }


    int len = input.size();
    size_t shape_len = array->arrayRef->shape_len;
    size_t* shape_array = new size_t[shape_len];
    for(size_t k = 0;k < shape_len;k++){
        if(k != dim){
            shape_array[k] = array->arrayRef->shape_array[k];
        }else{
            shape_array[k] = 0;
        }
        
    }
    print("shape",shape_array[0],shape_array[1],shape_array[2]);
    //size_t data_len = array->arrayRef->data_len * len;
    
    print("shape_len",shape_len);
    size_t* subscript_mul;
    for(const Array<value_type>& elem: input){
        for(size_t k = 0;k < shape_len;k++){
            if(k != dim){
                if(shape_array[k] != elem.arrayRef->shape_array[k]){
                    throw std::runtime_error("array shape musst be equal but dim = " + str(dim));
                }
            }else{
                shape_array[k] += elem.arrayRef->shape_array[k];
            }
        }
    }

    size_t data_len = 1;
    for(int k = 0; k < shape_len;k++){
        data_len = data_len* shape_array[k];
    }
    value_type* data_array = new value_type[data_len];
    
    size_t subscript_mul_depth = 1;
    for(int k = dim + 1; k < shape_len;k++){
        subscript_mul_depth = subscript_mul_depth * shape_array[k];
    }
    
    cat_assign_data(data_array,dim,subscript_mul_depth,array,len,shape_array[dim]);    


    auto out = Array<value_type>(shape_array,shape_len,data_array,data_len);
    return out;
    //for(int i = 0;i < shape_len; i++){
    //}
    //TODO dim != 0

}

