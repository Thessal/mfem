// Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-443211. All Rights
// reserved. See file COPYRIGHT for details.
//
// This file is part of the MFEM library. For more information and source code
// availability see http://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License (as published by the Free
// Software Foundation) version 2.1 dated February 1999.

#include "../../config/config.hpp"

#include "../../general/okina.hpp"
#include "../../linalg/kernels/vector.hpp"

#include "kGeometry.hpp"
#include "../fem.hpp"
#include "../doftoquad.hpp"

namespace mfem
{
// *****************************************************************************
#jit #kernel
void kGeom2D(const int NUM_DOFS_1D,
             const int NUM_QUAD_1D,
             const int numElements,
             const double* __restrict dofToQuadD,
             const double* __restrict nodes,
             double* __restrict J,
             double* __restrict invJ,
             double* __restrict detJ)
{
   const int NUM_DOFS = NUM_DOFS_1D*NUM_DOFS_1D;
   const int NUM_QUAD = NUM_QUAD_1D*NUM_QUAD_1D;
   MFEM_FORALL(e, numElements,
   {
      double s_nodes[2 * NUM_DOFS_1D * NUM_DOFS_1D];
      for (int q = 0; q < NUM_QUAD; ++q)
      {
         for (int d = q; d < NUM_DOFS; d +=NUM_QUAD)
         {
            s_nodes[ijN(0,d,2)] = nodes[ijkNM(0,d,e,2,NUM_DOFS)];
            s_nodes[ijN(1,d,2)] = nodes[ijkNM(1,d,e,2,NUM_DOFS)];
         }
      }
      for (int q = 0; q < NUM_QUAD; ++q)
      {
         double J11 = 0; double J12 = 0;
         double J21 = 0; double J22 = 0;
         for (int d = 0; d < NUM_DOFS; ++d)
         {
            const double wx = dofToQuadD[ijkNM(0,q,d,2,NUM_QUAD)];
            const double wy = dofToQuadD[ijkNM(1,q,d,2,NUM_QUAD)];
            const double x = s_nodes[ijN(0,d,2)];
            const double y = s_nodes[ijN(1,d,2)];
            J11 += (wx * x); J12 += (wx * y);
            J21 += (wy * x); J22 += (wy * y);
         }
         const double r_detJ = (J11 * J22)-(J12 * J21);
         assert(r_detJ!=0.0);
         J[ijklNM(0,0,q,e,2,NUM_QUAD)] = J11;
         J[ijklNM(1,0,q,e,2,NUM_QUAD)] = J12;
         J[ijklNM(0,1,q,e,2,NUM_QUAD)] = J21;
         J[ijklNM(1,1,q,e,2,NUM_QUAD)] = J22;
         const double r_idetJ = 1.0 / r_detJ;
         invJ[ijklNM(0,0,q,e,2,NUM_QUAD)] =  J22 * r_idetJ;
         invJ[ijklNM(1,0,q,e,2,NUM_QUAD)] = -J12 * r_idetJ;
         invJ[ijklNM(0,1,q,e,2,NUM_QUAD)] = -J21 * r_idetJ;
         invJ[ijklNM(1,1,q,e,2,NUM_QUAD)] =  J11 * r_idetJ;
         detJ[ijN(q,e,NUM_QUAD)] = r_detJ;
      }
   });
}

// *****************************************************************************
#jit #kernel
void kGeom3D(const int NUM_DOFS_1D,
             const int NUM_QUAD_1D,
             const int numElements,
             const double* __restrict dofToQuadD,
             const double* __restrict nodes,
             double* __restrict J,
             double* __restrict invJ,
             double* __restrict detJ)
{
   const int NUM_DOFS = NUM_DOFS_1D*NUM_DOFS_1D*NUM_DOFS_1D;
   const int NUM_QUAD = NUM_QUAD_1D*NUM_QUAD_1D*NUM_QUAD_1D;
   MFEM_FORALL(e,numElements,
   {
      double s_nodes[3 * NUM_DOFS_1D * NUM_DOFS_1D * NUM_DOFS_1D];
      for (int q = 0; q < NUM_QUAD; ++q)
      {
         for (int d = q; d < NUM_DOFS; d += NUM_QUAD)
         {
            s_nodes[ijN(0,d,3)] = nodes[ijkNM(0,d,e,3,NUM_DOFS)];
            s_nodes[ijN(1,d,3)] = nodes[ijkNM(1,d,e,3,NUM_DOFS)];
            s_nodes[ijN(2,d,3)] = nodes[ijkNM(2,d,e,3,NUM_DOFS)];
         }
      }
      for (int q = 0; q < NUM_QUAD; ++q)
      {
         double J11 = 0; double J12 = 0; double J13 = 0;
         double J21 = 0; double J22 = 0; double J23 = 0;
         double J31 = 0; double J32 = 0; double J33 = 0;
         for (int d = 0; d < NUM_DOFS; ++d)
         {
            const double wx = dofToQuadD[ijkNM(0,q,d,3,NUM_QUAD)];
            const double wy = dofToQuadD[ijkNM(1,q,d,3,NUM_QUAD)];
            const double wz = dofToQuadD[ijkNM(2,q,d,3,NUM_QUAD)];
            const double x = s_nodes[ijN(0,d,3)];
            const double y = s_nodes[ijN(1,d,3)];
            const double z = s_nodes[ijN(2,d,3)];
            J11 += (wx * x); J12 += (wx * y); J13 += (wx * z);
            J21 += (wy * x); J22 += (wy * y); J23 += (wy * z);
            J31 += (wz * x); J32 += (wz * y); J33 += (wz * z);
         }
         const double r_detJ = ((J11 * J22 * J33) + (J12 * J23 * J31) +
                                (J13 * J21 * J32) - (J13 * J22 * J31) -
                                (J12 * J21 * J33) - (J11 * J23 * J32));
         assert(r_detJ!=0.0);
         J[ijklNM(0,0,q,e,3,NUM_QUAD)] = J11;
         J[ijklNM(1,0,q,e,3,NUM_QUAD)] = J12;
         J[ijklNM(2,0,q,e,3,NUM_QUAD)] = J13;
         J[ijklNM(0,1,q,e,3,NUM_QUAD)] = J21;
         J[ijklNM(1,1,q,e,3,NUM_QUAD)] = J22;
         J[ijklNM(2,1,q,e,3,NUM_QUAD)] = J23;
         J[ijklNM(0,2,q,e,3,NUM_QUAD)] = J31;
         J[ijklNM(1,2,q,e,3,NUM_QUAD)] = J32;
         J[ijklNM(2,2,q,e,3,NUM_QUAD)] = J33;

         const double r_idetJ = 1.0 / r_detJ;
         invJ[ijklNM(0,0,q,e,3,NUM_QUAD)] = r_idetJ * ((J22 * J33)-(J23 * J32));
         invJ[ijklNM(1,0,q,e,3,NUM_QUAD)] = r_idetJ * ((J32 * J13)-(J33 * J12));
         invJ[ijklNM(2,0,q,e,3,NUM_QUAD)] = r_idetJ * ((J12 * J23)-(J13 * J22));
         invJ[ijklNM(0,1,q,e,3,NUM_QUAD)] = r_idetJ * ((J23 * J31)-(J21 * J33));
         invJ[ijklNM(1,1,q,e,3,NUM_QUAD)] = r_idetJ * ((J33 * J11)-(J31 * J13));
         invJ[ijklNM(2,1,q,e,3,NUM_QUAD)] = r_idetJ * ((J13 * J21)-(J11 * J23));
         invJ[ijklNM(0,2,q,e,3,NUM_QUAD)] = r_idetJ * ((J21 * J32)-(J22 * J31));
         invJ[ijklNM(1,2,q,e,3,NUM_QUAD)] = r_idetJ * ((J31 * J12)-(J32 * J11));
         invJ[ijklNM(2,2,q,e,3,NUM_QUAD)] = r_idetJ * ((J11 * J22)-(J12 * J21));
         detJ[ijN(q, e,NUM_QUAD)] = r_detJ;
      }
   });
}

// *****************************************************************************
void kGeom(const int DIM,
           const int NUM_DOFS,
           const int NUM_QUAD,
           const int numElements,
           const double* dofToQuadD,
           const double* nodes,
           double* J,
           double* invJ,
           double* detJ)
{
   const unsigned int dofs1D = IROOT(DIM,NUM_DOFS);
   const unsigned int quad1D = IROOT(DIM,NUM_QUAD);
   if (DIM==1) mfem_error("Not implemented");
   if (DIM==2)
      return kGeom2D(dofs1D, quad1D, numElements,
                     dofToQuadD, nodes, J, invJ, detJ);
   if (DIM==3)
      return kGeom3D(dofs1D, quad1D, numElements,
                     dofToQuadD, nodes, J, invJ, detJ);
   mfem_error("Should not be here!");
}

// *****************************************************************************
static kGeometry *geom = NULL;

// ***************************************************************************
// * ~ kGeometry
// ***************************************************************************
kGeometry::~kGeometry()
{
   free(geom->meshNodes);
   free(geom->J);
   free(geom->invJ);
   free(geom->detJ);
   delete[] geom;
}


// *****************************************************************************
#jit #kernel
static void kGeomFill(const int dims,
                      const size_t elements, const size_t numDofs,
                      const int* elementMap, int* eMap,
                      const double *nodes, double *meshNodes)
{
   MFEM_FORALL(e, elements,
   {
      for (size_t d = 0; d < numDofs; ++d)
      {
         const int lid = d+numDofs*e;
         const int gid = elementMap[lid];
         eMap[lid] = gid;
         for (int v = 0; v < dims; ++v)
         {
            const int moffset = v+dims*lid;
            const int xoffset = v+dims*gid;
            meshNodes[moffset] = nodes[xoffset];
         }
      }
   });
}

// *****************************************************************************
__kernel static void kArrayAssign(const int n, const int *src, int *dest)
{
   MFEM_FORALL(i, n, dest[i] = src[i];);
}

// *****************************************************************************
static void rNodeCopyByVDim(const int elements,
                            const int numDofs,
                            const int ndofs,
                            const int dims,
                            const int* eMap,
                            const double* Sx,
                            double* nodes)
{
   MFEM_FORALL(e,elements,
   {
      for (int dof = 0; dof < numDofs; ++dof)
      {
         const int lid = dof+numDofs*e;
         const int gid = eMap[lid];
         for (int v = 0; v < dims; ++v)
         {
            const int moffset = v+dims*lid;
            const int voffset = gid+v*ndofs;
            nodes[moffset] = Sx[voffset];
         }
      }
   });
}


// *****************************************************************************
kGeometry* kGeometry::Get(const FiniteElementSpace& fes,
                          const IntegrationRule& ir,
                          const Vector& Sx)
{
   const Mesh *mesh = fes.GetMesh();
   const GridFunction *nodes = mesh->GetNodes();
   const FiniteElementSpace *fespace = nodes->FESpace();
   const FiniteElement *fe = fespace->GetFE(0);
   const int dims     = fe->GetDim();
   const int numDofs  = fe->GetDof();
   const int numQuad  = ir.GetNPoints();
   const int elements = fespace->GetNE();
   const int ndofs    = fespace->GetNDofs();
   const kDofQuadMaps* maps = kDofQuadMaps::GetSimplexMaps(*fe, ir);
   rNodeCopyByVDim(elements,numDofs,ndofs,dims,geom->eMap,Sx,geom->meshNodes);
   kGeom(dims, numDofs, numQuad, elements,
         maps->dofToQuadD,
         geom->meshNodes, geom->J, geom->invJ, geom->detJ);
   return geom;
}

// *****************************************************************************
kGeometry* kGeometry::Get(const FiniteElementSpace& fes,
                          const IntegrationRule& ir)
{
   Mesh *mesh = fes.GetMesh();
   const bool geom_to_allocate = !geom;
   if (geom_to_allocate) { geom = new kGeometry(); }
   if (!mesh->GetNodes())
   {
      // mesh->SetCurvature(1, false, -1, Ordering::byVDIM);
   }
   const GridFunction *nodes = mesh->GetNodes();

   const mfem::FiniteElementSpace *fespace = nodes->FESpace();
   const mfem::FiniteElement *fe = fespace->GetFE(0);
   const int dims     = fe->GetDim();
   const int elements = fespace->GetNE();
   const int numDofs  = fe->GetDof();
   const int numQuad  = ir.GetNPoints();
   const bool orderedByNODES = (fespace->GetOrdering() == Ordering::byNODES);
   if (orderedByNODES) { ReorderByVDim(nodes); }
   const int asize = dims*numDofs*elements;
   mfem::Array<double> meshNodes(asize);
   const Table& e2dTable = fespace->GetElementToDofTable();
   const int* elementMap = e2dTable.GetJ();
   mfem::Array<int> eMap(numDofs*elements);
   kGeomFill(dims,
             elements,
             numDofs,
             elementMap,
             eMap.GetData(),
             nodes->GetData(),
             meshNodes.GetData());

   if (geom_to_allocate)
   {
      geom->meshNodes.allocate(dims, numDofs, elements);
      geom->eMap.allocate(numDofs, elements);
   }
   kVectorAssign(asize, meshNodes.GetData(), geom->meshNodes);
   kArrayAssign(numDofs*elements, eMap.GetData(), geom->eMap);
   // Reorder the original gf back
   if (orderedByNODES) { ReorderByNodes(nodes); }
   if (geom_to_allocate)
   {
      geom->J.allocate(dims, dims, numQuad, elements);
      geom->invJ.allocate(dims, dims, numQuad, elements);
      geom->detJ.allocate(numQuad, elements);
   }
   const kDofQuadMaps* maps = kDofQuadMaps::GetSimplexMaps(*fe, ir);
   kGeom(dims, numDofs, numQuad, elements, maps->dofToQuadD,
         geom->meshNodes, geom->J, geom->invJ, geom->detJ);
   return geom;
}

// ***************************************************************************
void kGeometry::ReorderByVDim(const GridFunction *nodes)
{
   const mfem::FiniteElementSpace *fes = nodes->FESpace();
   const int size = nodes->Size();
   const int vdim = fes->GetVDim();
   const int ndofs = fes->GetNDofs();
   double *data = nodes->GetData();
   double *temp = new double[size];
   int k=0;
   for (int d = 0; d < ndofs; d++)
      for (int v = 0; v < vdim; v++)
      {
         temp[k++] = data[d+v*ndofs];
      }
   for (int i = 0; i < size; i++)
   {
      data[i] = temp[i];
   }
   delete [] temp;
}

// ***************************************************************************
void kGeometry::ReorderByNodes(const GridFunction *nodes)
{
   const mfem::FiniteElementSpace *fes = nodes->FESpace();
   const int size = nodes->Size();
   const int vdim = fes->GetVDim();
   const int ndofs = fes->GetNDofs();
   double *data = nodes->GetData();
   double *temp = new double[size];
   int k = 0;
   for (int j = 0; j < ndofs; j++)
      for (int i = 0; i < vdim; i++)
      {
         temp[j+i*ndofs] = data[k++];
      }
   for (int i = 0; i < size; i++)
   {
      data[i] = temp[i];
   }
   delete [] temp;
}

}
