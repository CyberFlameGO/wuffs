// Copyright 2017 The Wuffs Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// +build ignore

package main

// gen.go converts base.* to data.go.
//
// Invoke it via "go generate".

import (
	"bytes"
	"fmt"
	"go/format"
	"io/ioutil"
	"os"
)

const columns = 1024

func main() {
	if err := main1(); err != nil {
		os.Stderr.WriteString(err.Error() + "\n")
		os.Exit(1)
	}
}

func main1() error {
	out := &bytes.Buffer{}
	out.WriteString("// Code generated by running \"go generate\". DO NOT EDIT.\n")
	out.WriteString("\n")
	out.WriteString("// Copyright 2017 The Wuffs Authors.\n")
	out.WriteString("//\n")
	out.WriteString("// Licensed under the Apache License, Version 2.0 (the \"License\");\n")
	out.WriteString("// you may not use this file except in compliance with the License.\n")
	out.WriteString("// You may obtain a copy of the License at\n")
	out.WriteString("//\n")
	out.WriteString("//    https://www.apache.org/licenses/LICENSE-2.0\n")
	out.WriteString("//\n")
	out.WriteString("// Unless required by applicable law or agreed to in writing, software\n")
	out.WriteString("// distributed under the License is distributed on an \"AS IS\" BASIS,\n")
	out.WriteString("// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n")
	out.WriteString("// See the License for the specific language governing permissions and\n")
	out.WriteString("// limitations under the License.\n")
	out.WriteString("\n")
	out.WriteString("package cgen\n")
	out.WriteString("\n")

	if err := genBase(out); err != nil {
		return err
	}

	formatted, err := format.Source(out.Bytes())
	if err != nil {
		return err
	}
	return ioutil.WriteFile("data.go", formatted, 0644)
}

func genBase(out *bytes.Buffer) error {
	files := []struct {
		filename, varname string
	}{
		{"base/all-impl.c", "baseAllImplC"},
		{"base/image-impl.c", "baseImageImplC"},

		{"base/core-private.h", "baseCorePrivateH"},
		{"base/core-public.h", "baseCorePublicH"},
		{"base/memory-private.h", "baseMemoryPrivateH"},
		{"base/memory-public.h", "baseMemoryPublicH"},
		{"base/image-private.h", "baseImagePrivateH"},
		{"base/image-public.h", "baseImagePublicH"},
		{"base/io-private.h", "baseIOPrivateH"},
		{"base/io-public.h", "baseIOPublicH"},
		{"base/range-private.h", "baseRangePrivateH"},
		{"base/range-public.h", "baseRangePublicH"},
		{"base/token-private.h", "baseTokenPrivateH"},
		{"base/token-public.h", "baseTokenPublicH"},
	}

	prefixAfterEditing := []byte("// After editing this file,")
	prefixCopyright := []byte("// Copyright ")
	copyright := []byte(nil)

	for _, f := range files {
		in, err := ioutil.ReadFile(f.filename)
		if err != nil {
			return err
		}

		if !bytes.HasPrefix(in, prefixAfterEditing) {
			return fmt.Errorf("%s's contents do not start with %q", f.filename, prefixAfterEditing)
		}
		if i := bytes.Index(in, []byte("\n\n")); i >= 0 {
			in = in[i+2:]
		}

		if !bytes.HasPrefix(in, prefixCopyright) {
			return fmt.Errorf("%s's contents do not start with %q", f.filename, prefixCopyright)
		}
		if i := bytes.Index(in, []byte("\n\n")); i >= 0 {
			if len(copyright) == 0 {
				copyright = in[:i+2]
			}
			in = in[i+2:]
		}

		fmt.Fprintf(out, "const %s = \"\" +\n", f.varname)
		writeStringConst(out, in)
		out.WriteString("\"\"\n\n")
	}

	fmt.Fprintf(out, "const baseCopyright = \"\" +\n")
	writeStringConst(out, copyright)
	out.WriteString("\"\"\n\n")
	return nil
}

var dashDashDashDash = []byte("// ----")

func writeStringConst(out *bytes.Buffer, s []byte) {
	for len(s) > 0 {
		remaining := []byte(nil)
		if i := bytes.Index(s[1:], dashDashDashDash); i >= 0 {
			s, remaining = s[:i+1], s[i+1:]
		}

		for len(s) > 0 {
			t := s
			if len(t) > columns {
				t = t[:columns]
			}
			s = s[len(t):]
			fmt.Fprintf(out, "%q +\n", t)
		}

		s = remaining
		if len(s) > 0 {
			out.WriteString("\"\" +\n")
		}
	}
}
