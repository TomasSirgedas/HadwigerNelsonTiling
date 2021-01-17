#pragma once

#include "CoreMacros.h"
#include "DualGraph.h"
#include <string>
#include <vector>

class DualAnalysis
{
public:
   CORE_API DualAnalysis( const DualGraph& dual );

   CORE_API bool isValid() const { return _IsValid; }
   CORE_API std::string errorMessage() const { return _ErrorMessage; }
   CORE_API std::vector<DualGraph::VertexPtr> errorVertices() const { return _ErrorVertices; }

private:
   void setError( const std::string& error, std::vector<DualGraph::VertexPtr> vertices );

private:
   std::string _ErrorMessage = "this is an error message";
   std::vector<DualGraph::VertexPtr> _ErrorVertices;

private:
   bool _IsValid = false;
};

