// -*- mode: javascript -*-  vim: set ft=javascript:
{
  "Name": "precAFP3D",
  "ShortName":"precAFP3D",
  "Model":"linear",
  "Description":"Magneto Static",
  "Parameters":
  {
    "mu_0":
    {
      "type":"constant",
      "name":"Vacuum permeability",
      "value":12.5663706144e-7
    }
  },
  "Materials":
  {
    "firstMat": // Physical marker
    {
      "name":"firstMat",
      "file":"false",
      "B":"mu_r*mu_0*H:x:y:z:H:mu_0:B:mu_r"
    }
  }, // materials
  "BoundaryConditions":
  {
    "u":
    {
      "Dirichlet":
      {
        "Border":
        {
          "expr":"{0,0,0}:x:y:z"
          //"expr":"{pi*sin(pi*x)*cos(pi*y),-pi*cos(pi*x)*sin(pi*y)}:x:y"
        }
      }
    },
    "phi":
    {
      "Dirichlet":
      {
        "Border":
        {
          "expr":"0"
        }
      }
    }
  } // BoundaryConditions
}

