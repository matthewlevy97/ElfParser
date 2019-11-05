// Find at -> /usr/include/elf.h
#include <elf.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DEBUG 1

static int parse_elf32(unsigned char *e_ident, int fd);
static int parse_elf64(unsigned char *e_ident, int fd);

char * HEADER_TYPES[] = {
	"None",
	"Relocatable",
	"Executable",
	"Shared Object",
	"Core"
};

int parse_elf(int fd)
{
	unsigned char e_ident[EI_NIDENT];
	
	read(fd, e_ident, sizeof(e_ident));
	
	// Verify this is an ELF
	if(*(int*)e_ident != *(int*)ELFMAG) {
		printf("Not an ELF\n");
		return -1;
	}
	
	switch(e_ident[EI_CLASS]) {
		case ELFCLASSNONE: printf("Invalid ELF Class"); return -2;
		case ELFCLASS32:   parse_elf32(e_ident, fd); break;
		case ELFCLASS64:   parse_elf64(e_ident, fd); break;
	}
	
	return 0;
}

static int parse_elf32(unsigned char *e_ident, int fd)
{
	printf("32\n");
	return 0;
}
static int parse_elf64(unsigned char *e_ident, int fd)
{
	Elf64_Ehdr header;
	Elf64_Phdr * segment_headers;
	Elf64_Shdr * section_headers;
	
	memcpy(header.e_ident, e_ident, sizeof(header.e_ident));
	
	// Read rest of header
	read(fd, &header.e_type, sizeof(header) - sizeof(header.e_ident));
	
#ifdef DEBUG
	// Display Header
	printf("Type: %s\n", HEADER_TYPES[header.e_type]);
	printf("Architecture: %d\n", header.e_machine);
	printf("Version: %d\n", header.e_version);
	printf("Entry: 0x%08x\n", header.e_entry);
	printf("Program Header Table Offset: %u\n", header.e_phoff);
	printf("Section Header Table Offset: %u\n", header.e_shoff);
	printf("Header Size: %d\n", header.e_ehsize);
	printf("\t--------\n");
#endif
	
	// Load segment
	lseek(fd, header.e_phoff, SEEK_SET);
	segment_headers = malloc(sizeof(Elf64_Phdr) * header.e_phnum);

#ifdef DEBUG
	printf("\nSegments:\n");
#endif
	for(int i = 0; i < header.e_phnum; i++) {
		read(fd, &segment_headers[i], sizeof(Elf64_Phdr));
		
#ifdef DEBUG
		printf("Segment Type: 0x%08x\n", segment_headers[i].p_type);
		printf("File Offset: %u\n", segment_headers[i].p_offset);
		printf("Virtual Address: 0x%08x\n", segment_headers[i].p_vaddr);
		printf("Physical Address: 0x%08x\n", segment_headers[i].p_paddr);
		printf("Size in File: %u\n", segment_headers[i].p_filesz);
		printf("Size in Memory: %u\n", segment_headers[i].p_memsz);
		printf("\t--------\n");
#endif
	}
	
	// Load first section
	lseek(fd, header.e_shoff, SEEK_SET);
	section_headers = malloc(sizeof(Elf64_Shdr) * header.e_phnum);
	
	for(int i = 0; i < header.e_shnum; i++) {
		read(fd, &section_headers[i], sizeof(Elf64_Shdr));
	}
	
#ifdef DEBUG
	auto offset = section_headers[header.e_shstrndx].sh_offset;
	
	printf("\nSections:\n");
	for(int i = 0; i < header.e_shnum; i++) {
		// Display Section
		char name[1024];
		
		lseek(fd, offset + section_headers[i].sh_name, SEEK_SET);
		read(fd, name, sizeof(name));
		
		printf("Name: %s\n", name);
		printf("Type: 0x%08x\n", section_headers[i].sh_type);
		printf("Virtual Address: 0x%08x\n", section_headers[i].sh_addr);
		printf("File Offset: %d\n", section_headers[i].sh_offset);
		printf("Section Size: %d\n", section_headers[i].sh_size);
		printf("\t--------\n");

	}
#endif
	
	return 0;
}

int main(int argc, char ** argv)
{
	int fd;
	fd = open(argv[1], O_RDONLY);
	
	parse_elf(fd);
	
	close(fd);
	return 0;
}
