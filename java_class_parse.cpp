#include <cerrno>
#include <exception>
#include <fstream>
#include "java_class_parse.h"
#include <stdio.h>
#include <map>
#include <string.h>
#include <string>
#include <stdlib.h>


using namespace std;

int u1_func(u1* bytes,int &startidx)
{
	return (int)bytes[startidx++];
}

int byte1Leftshift_or_byte2_func(u1* bytes,int &startidx)
{
	return (int)bytes[startidx++]<<8 | bytes[startidx++];
}
//[)
const std::map<int,std::pair<const char *,int (*)(u1* bytes,int &startidx)>> opcode_map = {
	{0x19,{"aload",u1_func}},
	{0xb2,{"getstatic",byte1Leftshift_or_byte2_func}},
	{0x12,{"ldc",u1_func}},
	{0xb6,{"invokevirtual",byte1Leftshift_or_byte2_func}},
	{0xb8,{"invokestatic",byte1Leftshift_or_byte2_func}},
	{0xb1,{"return",u1_func}}
#if 0
	{0x2a,{"aload_0",0}},
	{0x2b,{"aload_1",0}},
	{0x2c,{"aload_2",0}},
	{0x2d,{"aload_3",0}},
	{0xb5,{"putfield",2}},
	{0xb1,{"return",0}},
	{0x00,{"nop",0}},
	{0x01,{"aconst_null",0}},
	
#endif
};


ClassFile::ClassFile(std::ifstream & f) {
	parse_header(f);
	parse_constant_pool(f);
	parse_classmsg(f);
	parse_interface(f);
	parse_field(f);
	parse_method(f);
}

void ClassFile::parse_attribute(std::ifstream &f)
{
	attributes_count = read2(f);
	if(attributes_count != 0)
	{
		attributes = new attribute_info*[attributes_count];
	}
	for(int j = 0; j < attributes_count; ++j)
	{
		u2 idx = read2(f);
		u4 len = read4(f);
		attribute_info* a_info = new_attribute(f,idx,len);		
		attributes[j] = a_info;
	}
}



void method_info::parse_descriptor(cp_utf8_info* info)
{

}

void ClassFile::parse_method(std::ifstream &f)
{
	methods_count = read2(f);
	if(methods_count != 0)
	{
		methods = new method_info*[methods_count];
	}
	printf("method count :%d\n",methods_count);
	for(int i = 0; i < methods_count; ++i)
	{
		method_info * info = new method_info;
		info->access_flags = read2(f);
		info->name_index = read2(f);
		u2 descriptor_index = read2(f);
		info->attributes_count = read2(f);

		cp_utf8_info* des_info = (cp_utf8_info*)constant_pool[descriptor_index];	
		info->parse_descriptor(des_info);

		if(info->attributes_count != 0)
		{
			info->attributes = new attribute_info*[info->attributes_count];
		}
		for(int j = 0; j < info->attributes_count; ++j)
		{
			u2 idx = read2(f);
			u4 len = read4(f);
			attribute_info* a_info = new_attribute(f,idx,len);
			info->attributes[j] = a_info;
		}
		methods[i] = info;
	}
}

void ClassFile::fill_method_acc(u2 flags,std::string &s)
{
	if(flags & ACC_PUBLIC)
	{
		s += "public ";
	}
	else if(flags & ACC_PRIVATE)
	{
		s += "private ";
	}
	else if(flags & ACC_PROTECTED)
	{
		s += "protected ";
	}
	else if(flags & ACC_STATIC)
	{
		s += "static ";
	}
	else if(flags & ACC_FINAL)
	{
		s += "final ";
	}
	else if(flags & ACC_SUPER)
	{
		s += "super "; 
	}
}

void ClassFile::fill_method_name_and_arg(std::string& s,method_info* info)
{
	}

void ClassFile::print_method(method_info* info)
{
	if(!info)
		return;
	std::string method_descriptor;
	printf("acc_tag:%d\n",info->access_flags);
	fill_method_acc(info->access_flags,method_descriptor);
	fill_method_name_and_arg(method_descriptor,info);	
	printf("%s\n",method_descriptor.c_str());
	//printf("name:%s\n",((cp_utf8_info*)(constant_pool[info->name_index]))->bytes);
	int cnt = 0;
	for(int i = 0 ; i < info->attributes_count; ++i)
	{
		attribute_info* ainfo = info->attributes[i];
		print_attribute(ainfo,1);
	}
	printf("\n\n");
}

void ClassFile::parse_field(std::ifstream &f)
{
	fields_count = read2(f);
	printf("fields_count : %d\n",fields_count);
	if(fields_count != 0)
	{
		fields = new field_info*[fields_count];			
	}
	for(int i = 0; i < fields_count; ++i)
	{
		field_info * info = new field_info;
		info->access_flags = read2(f);
		info->name_index = read2(f);
		info->descriptor_index = read2(f);
		info->attributes_count = read2(f);
		
		if(info->attributes_count != 0)
		{
			info->attributes = new attribute_info*[info->attributes_count];
		}
		for(int j = 0; j < info->attributes_count; ++j)
		{
			u2 idx = read2(f);
			u4 len = read4(f);
			attribute_info* a_info = new_attribute(f,idx,len);		
			info->attributes[i] = a_info;
		}
		fields[i] = info;
	}
}

void ClassFile::parse_interface(std::ifstream &f)
{
	interfaces_count = read2(f);
	if(interfaces_count)
	{
		interfaces = new u2[interfaces_count];
	}
	for(int i = 0; i < interfaces_count; ++i)
	{
		interfaces[i] = read2(f);
	}
}

void ClassFile::parse_classmsg(std::ifstream &f)
{
	access_flags = read2(f);
	this_class = read2(f);
	super_class = read2(f);
}

void ClassFile::parse_constant_pool(std::ifstream &f)
{
	u2 constant_pool_count = read2(f);
	printf("constant pool size:%d\n",constant_pool_count);
	for(int i = 1; i < constant_pool_count; ++i)
	{
		u1 tag = read1(f);
		switch(tag)
		{
			case CONSTANT_Class:
			{
				cp_class_info* info = new cp_class_info;
				info->tag = tag;
				info->name_index = read2(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_Fieldref:
			case CONSTANT_Methodref:
			case CONSTANT_InterfaceMethodref:
			{
				cp_fmi_info* info = new cp_fmi_info;
				info->tag = tag;
				info->class_index = read2(f);
				info->name_and_type_index = read2(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_String:
			{
				cp_string_info* info = new cp_string_info;
				info->tag = tag;
				info->string_index = read2(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_Integer:
			{
				cp_interger_info* info = new cp_interger_info;
				info->tag = tag;
				info->bytes = read4(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_Float:
			{
				cp_float_info* info = new cp_float_info;
				info->tag = tag;
				info->bytes = read4(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_Long:
			{
				cp_long_info* info = new cp_long_info;
				info->tag = tag;
				info->high_bytes = read4(f);
				info->low_bytes = read4(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_Double:
			{
				cp_double_info* info = new cp_double_info;
				info->tag = tag;
				info->high_bytes = read4(f);
				info->low_bytes = read4(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_NameAndType:
			{
				cp_name_and_type_info *info = new cp_name_and_type_info;
				info->tag = tag;
				info->name_index = read2(f);
				info->descriptor_index = read2(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_Utf8:
			{
				cp_utf8_info * info = new cp_utf8_info;
				info->tag = tag;
				info->length = read2(f);
				info->bytes = new u1[info->length];
				f.read((char*)info->bytes,sizeof(u1)*info->length);
				//printf("[%s]\n",info->bytes);	
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_MethodHandle:
			{
				cp_method_handle_info *info = new cp_method_handle_info;
				info->tag = tag;
				info->reference_kind = read1(f);
				info->reference_index = read2(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_MethodType:
			{
				cp_method_type_info* info = new cp_method_type_info;
				info->tag = tag;
				info->descriptor_index = read2(f);
				constant_pool[i] = info;
			}
			break;
			case CONSTANT_InvokeDynamic:
			{
				cp_invoke_dynamic_info * info = new cp_invoke_dynamic_info;
				info->tag = tag;
				info->bootstrap_method_attr_index = read2(f);
				info->name_and_type_index = read2(f);
				constant_pool[i] = info;
			}
			break;
			default:
			{
				printf("undefine constant tag! %d\n",tag);
				//exit(1);
			}
			break;
		}
	}
}

void ClassFile::parse_header(std::ifstream & f)
{
	u4 magic = read4(f);
	if(magic != MAGIC_NUMBER)
	{
		cout<<"magic_num wrong! cur_num:"<<
			hex<<magic<<",mgnum:"<<MAGIC_NUMBER<<endl;
		return;
	}
	u2 minor_version = read2(f);
	u2 major_version = read2(f);
#if DEBUG_TAG
	cout<<hex<<"ver:"<<minor_version<<"."<<major_version<<endl;
#endif
}

u1 read1(std::ifstream& f)
{
	u1 readnum;
	f.read((char *)&readnum,sizeof(readnum));
	return readnum;
}

u2 read2(std::ifstream& f)
{
	u2 readnum;
	f.read((char *)&readnum,sizeof(readnum));
	return ntole(readnum);
}

u4 read4(std::ifstream& f)
{
	u4 readnum;
	f.read((char *)&readnum,sizeof(readnum));
	return ntole(readnum);
}
ClassFile::~ClassFile()
{}


method_info* lookup_method(ClassFile &cf,const char * str)
{
	for(int i = 0; i < cf.methods_count; ++i)
	{
		method_info* info = cf.methods[i];
		cp_utf8_info* _info = (cp_utf8_info*)cf.constant_pool[info->name_index];	
		if(!_info)
			continue;
		if(strcmp(str,(const char *)_info->bytes) == 0)
		{
			printf(" find method: %s\n",_info->bytes);
			return info;
		}
	}
	printf("cannot find %s\n",str);
	return nullptr;
}

exception_info* ClassFile::new_exception(std::ifstream &f)
{
	exception_info * info = new exception_info;
	info->start_pc = read2(f);
	info->end_pc = read2(f);
	info->handler_pc = read2(f);
	info->catch_type = read2(f);
	return info;
}

attribute_info* ClassFile::new_attribute(std::ifstream &f,u2 nameidx,u4 len)
{
	cp_utf8_info* _info = (cp_utf8_info*)constant_pool[nameidx];
	const char * cmp = (const char *)_info->bytes;
	attribute_info* ret_info = nullptr;

	if(!strcmp(cmp,"Code"))
	{
		attribute_code_info * code_info = new attribute_code_info;
		code_info->attribute_name_index = nameidx;
		code_info->attribute_length = len;
		code_info->max_stack = read2(f);
		code_info->max_locals = read2(f);
		code_info->code_length = read4(f);
		code_info->code = new u1[code_info->code_length];
		f.read((char *)code_info->code,sizeof(u1)*code_info->code_length);
		code_info->exception_table_length = read2(f);
		code_info->exception_table = new exception_info*[code_info->exception_table_length];
		for(int i = 0; i < code_info->exception_table_length ; ++i)
		{
			code_info->exception_table[i] = new_exception(f);
		}
		code_info->attributes_count = read2(f);
		if(code_info->attributes_count != 0)
		{
			code_info->attributes = new attribute_info*[code_info->attributes_count];
		}
		for(int j = 0; j < code_info->attributes_count; ++j)
		{
			u2 idx = read2(f);
			u4 len = read4(f);
			attribute_info* a_info = new_attribute(f,idx,len);		
			code_info->attributes[j] = a_info;
		}
		ret_info = (attribute_info *)code_info;
		//print_attribute(ret_info,1);
	}
	else 
	{
		ret_info = new attribute_info;
		ret_info->attribute_name_index = nameidx;
		ret_info->attribute_length = len;
		u1 buffer[ret_info->attribute_length];
		f.read((char *)buffer,sizeof(u1) * ret_info->attribute_length);
	}
#if 0
	if(!strcmp(cmp,"SourceFile"))
	{

	}
	else if(!strcmp(cmp,"InnerClasses"))
	{

	}	
	else if(!strcmp(cmp,"EnclosingMethod"))
	{

	}
	else if(!strcmp(cmp,"SourceDebugExtension"))
	{

	}
	else if(!strcmp(cmp,"BootstrapMethods"))
	{

	}
	else if(!strcmp(cmp,"ConstantValue"))
	{

	}	
	else if(!strcmp(cmp,"Exceptions"))
	{

	}
	else if(!strcmp(cmp,"RuntimeVisibleParameterAnnotations"))
	{

	}
	else if(!strcmp(cmp,"RuntimeInvisibleParameterAnnotations"))
	{

	}
	else if(!strcmp(cmp,"AnnotationDefault"))
	{

	}	
	else if(!strcmp(cmp,"MethodParameters"))
	{

	}
	else if(!strcmp(cmp,"Synthetic"))
	{

	}
	else if(!strcmp(cmp,"Deprecated"))
	{

	}
	else if(!strcmp(cmp,"Signature"))
	{

	}	
	else if(!strcmp(cmp,"RuntimeVisibleAnnotations"))
	{

	}
	else if(!strcmp(cmp,"RuntimeInvisibleAnnotations"))
	{

	}
	else if(!strcmp(cmp,"LineNumberTable"))
	{

	}	
	else if(!strcmp(cmp,"LocalVariableTable"))
	{

	}
	else if(!strcmp(cmp,"LocalVariableTypeTable"))
	{

	}
	else if(!strcmp(cmp,"StackMapTable"))
	{

	}
	else if(!strcmp(cmp,"RuntimeVisibleTypeAnnotations"))
	{

	}	
	else if(!strcmp(cmp,"RuntimeInvisibleTypeAnnotations"))
	{

	}
	else 
	{
		printf("unsupport attributes :%s!\n",cmp);
	}
#endif
	return ret_info;
}


void ClassFile::print_code(u1 *code,u4 len)
{
	int idx = 0;
	while(idx < len)
	{
		u1 tag = code[idx++];
		auto it = opcode_map.find(tag);
		if(it == opcode_map.end())
		{
			printf("	opcode:%x is not supported.\n",tag);
			continue;
		}
		auto ret = it->second.second(code,idx);
		int charlen = strlen(it->second.first);
		charlen = 20 - charlen;
		std::string s;
		while(charlen--)
		{
			s+=' ';
		}
		if(ret)
			printf("	%s :%s%d\n",it->second.first,s.c_str(),ret);
		else 
		{
			printf("	%s\n",it->second.first);
		}
	}
}

void ClassFile::print_attribute(attribute_info* info,u1 tag)
{
	if(!info)
	{
		return;
	}
	switch(tag)
	{
		case ATT_CODE:
		{
			printf("  Code:\n");
			attribute_code_info* cinfo = (attribute_code_info*)info;
			print_code(cinfo->code,cinfo->code_length); 
		}
		break;
		default:
		{}
		break;
	}
}

