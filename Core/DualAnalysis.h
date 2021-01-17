#pragma once

#include "CoreMacros.h"
#include "DualGraph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class DualAnalysis
{
public:
   CORE_API DualAnalysis( const DualGraph& dual );

   CORE_API bool isValid() const { return _IsValid; }
   CORE_API std::string errorMessage() const { return _ErrorMessage; }
   CORE_API std::vector<DualGraph::VertexPtr> errorVertices() const { return _ErrorVertices; }
   CORE_API bool isCurved( const DualGraph::VertexPtr& a, const DualGraph::VertexPtr& b ) const;
   CORE_API bool isCurvedTowardsA( const DualGraph::VertexPtr& a, const DualGraph::VertexPtr& b ) const;
   CORE_API bool isError() { return !_ErrorMessage.empty(); }

private:
   void init( const DualGraph& dual );
   void setError( const std::string& error, std::vector<DualGraph::VertexPtr> vertices );

private:
   std::string _ErrorMessage;
   std::vector<DualGraph::VertexPtr> _ErrorVertices;

private:
   std::map<std::pair<int,int>, bool> _CurveDirectionForEdge;

private:
   bool _IsValid = false;
};

