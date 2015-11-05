// -*- mode: javascript -*-  vim: set ft=javascript:
{
  "Name": "Torus Quart",
  "ShortName":"tq",
  "Model":"magnetostatique",
  "Description":"Magnetostatique",
  "Parameters":
  {
    "a":
    {
      "type":"constant",
      "name":"model parameter 1",
      "value":1
    },
    "b":
    {
      "type":"constant",
      "name":"model parameter 2",
      "value":5e-3
    },
    "c":
    {
      "type":"constant",
      "name":"model parameter 3",
      "value":5e-3
    }
  },
  "Materials":
  {
    "COIL": // Physical marker
    {
      "name":"COIL", // Name of material
      "B": "mu_r*mu_0*H:x:y:H:mu_0:mu_r:B",
      "file" : "false",
      "j":"{-48.e+6*(0.5/(2*Pi))*y/(x^2+y^2),48.e+6*(0.5/(2*Pi))*x/(x^2+y^2),0}:x:y:z",
      "ex":"1"
    },
    "Omega":
    {
      "name":"Omega",
      "B": "mu_r*mu_0*H:x:y:H:mu_0:mu_r:B",
      "file":"false",
      "j":"0",
      "ex":"1"
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
        }
      }
    }
  }
}

