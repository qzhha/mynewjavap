#pragma once
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>

typedef std::uint8_t u1;
typedef std::uint16_t u2;
typedef std::uint32_t u4;


#define DEBUG_TAG true

#define MAGIC_NUMBER 0xCAFEBABE

//constant pool tag
#define CONSTANT_Class 7
#define CONSTANT_Fieldref 9
#define CONSTANT_Methodref 10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_String 8
#define CONSTANT_Integer 3
#define CONSTANT_Float 4 
#define CONSTANT_Long 5
#define CONSTANT_Double 6
#define CONSTANT_NameAndType 12
#define CONSTANT_Utf8 1
#define CONSTANT_MethodHandle 15
#define CONSTANT_MethodType 16
#define CONSTANT_InvokeDynamic 18


//ACC TAG
#define ACC_PUBLIC			0x0001
#define ACC_PRIVATE			0x0002
#define ACC_PROTECTED		0x0004
#define ACC_STATIC			0x0008
#define ACC_FINAL			0x0010
#define ACC_SUPER			0x0020

//Method access and property flags
#define ACC_SYNCHRONIZED    0x0020
#define ACC_BRIDGE			0x0040
#define ACC_VARARGS			0x0080
#define ACC_NATIVE			0x0100
//.Field access and property flags
#define ACC_VOLATILE		0x0040
#define ACC_TRANSIENT		0x0080

#define ACC_INTERFACE		0x0200
#define ACC_ABSTRACT		0x0400
#define ACC_SYNTHETIC		0x1000
#define ACC_ANNOTATION		0x2000
#define ACC_ENUM			0x4000


struct attribute_info;

struct cp_info {
	u1 tag;
};

struct cp_class_info : public cp_info{
	u2	name_index;	
};

struct cp_utf8_info : public cp_info{
	u2 length;
	u1 *bytes;
};

struct cp_fmi_info: public cp_info {
	u2 class_index;
	u2 name_and_type_index;
};

struct cp_string_info : public cp_info {
	u2 string_index;
};

struct cp_interger_info : public cp_info {
	u4 bytes;
};
struct cp_float_info : public cp_info {
	u4 bytes;
};
struct cp_long_info : public cp_info {
	u4 high_bytes;
	u4 low_bytes;
};
struct cp_double_info : public cp_info {
	u4 high_bytes;
	u4 low_bytes;
};

struct cp_name_and_type_info : public cp_info {
	u2 name_index;
	u2 descriptor_index;
};

struct cp_method_handle_info : public cp_info {
	u1 reference_kind;
	u2 reference_index;
};

struct cp_method_type_info: public cp_info {
	u2 descriptor_index;
};

struct cp_invoke_dynamic_info : public cp_info {
	u2 bootstrap_method_attr_index;
	u2 name_and_type_index;
};

struct field_info {
	u2	access_flags;
	u2  name_index;
	u2  descriptor_index;
	u2  attributes_count;
	attribute_info **attributes;
};


u1 read1(std::ifstream& f);
u2 read2(std::ifstream& f);
u4 read4(std::ifstream& f);


template <class T>
T ntole(T num)
{
	T ret = 0;
	unsigned char * p = (unsigned char*)&num;
	for(int i = 0; i < sizeof(T); i++)
		ret |= (p[i] << ((sizeof(T) - i - 1) * 8));
	return ret;
}

struct exception_info {
	u2	start_pc;
	u2  end_pc;
	u2  handler_pc;
	u2  catch_type;
};

struct method_info {
	u2  access_flags;
	u2  name_index;
	u2  descriptor_index;
	u2  attributes_count;
	attribute_info **attributes;
};

struct attribute_info {
	u2  attribute_name_index;
	u4  attribute_length;
}; 

struct attribute_constantvalue_info:public attribute_info {
	u2  constantvalue_index;	
};

struct attribute_code_info: public attribute_info {
	u2 attribute_name_index;
	u4 attribute_length;
	u2 max_stack;
	u2 max_locals;
	u4 code_length;
	u1* code;
	u2 exception_table_length;
	exception_info **exception_table;
	u2 attributes_count;
	attribute_info **attributes;
};


class ClassFile;




method_info* lookup_method(ClassFile &cf,const char * str = "");

class ClassFile {
public:
	u4 magic;
	u2 minor_version;
	u2 major_version;
	u2 constant_pool_count;
	cp_info **constant_pool;
	u2 access_flags;
	u2 this_class;
	u2 super_class;
	u2 interfaces_count;
	u2 *interfaces;
	u2 fields_count;
	field_info **fields;
	u2 methods_count;
	method_info **methods;
	u2 attributes_count;
	attribute_info **attributes;


	enum {
		ATT_CODE = 1,
	};

	
	void fill_method_acc(u2 flags,std::string& s);
	void fill_method_name_and_arg(std::string& s,method_info* info);
	void print_method(method_info* info);
	void print_attribute(attribute_info* info,u1 tag);
	void print_code(u1 *code,u4 code_len);


	attribute_info* new_attribute(std::ifstream &f,u2 nameidx,u4 len);
	exception_info* new_exception(std::ifstream &f);
	//
	void parse_header(std::ifstream &f);
	void parse_constant_pool(std::ifstream &f);
	void parse_classmsg(std::ifstream &f);
	void parse_interface(std::ifstream &f);
	void parse_field(std::ifstream &f);
	void parse_method(std::ifstream &f);
	void parse_attribute(std::ifstream &f);


	ClassFile() {}
	ClassFile(std::ifstream &f);
	ClassFile(const ClassFile& f) = delete;
	~ClassFile();
};

