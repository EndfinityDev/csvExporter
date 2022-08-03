/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
************************************************** */

#pragma once

#include "csvExporter.h"

class AuExpMesh
{
public:

	static void SaveMeshFile(const AuCarExpMesh* mesh, const wchar_t* filename);
};
