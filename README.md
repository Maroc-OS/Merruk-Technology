Merruk Technology Kernel v.2.0 For Samsung Galaxy Y 'Young' GT-S5360.

How to use
==============

Compiling
---------------

For compiling Merruk Technology Kernel open the command prompt (CONSOLE) and see this :

<table>
  <tr>
	<th>Description</th><th>Meaning</th><th>Command</th><th>Parameter</th>
  </tr>
  <tr>
	<td>New Compile/Update Existing Kernel</td><td>First Compile or Only Modded Codes</td><td>Kernel_Make</td><td>-MT</td>
  </tr>
  <tr>
	<td>Specific CONFIG_FILE Compile</td><td>Must be in '/arch/arm/configs/'</td><td>Kernel_Make</td><td>-CF totoro_brcm21553_05_defconfig</td>
  </tr>
  <tr>
	<td>Clean Build (0 files already built)</td><td>Like first time, clean files and build</td><td>Kernel_Make</td><td>-CL</td>
  </tr>
  <tr>
	<td>Specify number of prosessors</td><td>Script guesses that automatically</td><td>Kernel_Make</td><td>-CPU [Number of cores] (all cores by default)</td>
  </tr>
</table>


Preparing the environment
----------------------------

On first use of the "Tools" directory you need to set up the environment :

Usage :

	cd ./Tools

	./Install.sh [Parameter]

Parameters:
<dl>
	<dt>merruk</dt>
	<dd>Use Merruk Technology InitRamFS</dd>
	<dt>stock</dt>
	<dd>Use Samsung stock InitRamFS</dd>
</dl>



Kernel compression
--------------------

You can simply call this tool after you made changes in the boot directory (RamFS) :

How To Use :

	cd ./Tools

	./Compress.sh [Parameter]

Parameters:
<dl>
	<dt>merruk</dt>
	<dd>Use Merruk Technology kernel</dd>
	<dt>stock</dt>
	<dd>Use Samsung stock kernel</dd>
</dl>


Note:
------

Your compressed kernel will be built as:

<table>
	<tr>
		<td>PDA.[parameter].tar</td><td>Odin file</td>
		<td>Kernel.[parameter].Boot.img</td><td>Raw file, Use With 'dd' Command</td>
	</tr>
</table>