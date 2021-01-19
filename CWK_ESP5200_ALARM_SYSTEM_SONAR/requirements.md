- Your coursework application must have a RT timing / behavioural requirement.

- Explicitly specify the actual timing constraints (e.g. sampling intervals for temperature, or a time-limit for entering a key-code).

- You need to be able to test your system for conformance to the timing requirements.

- Implement an embedded control system for a RT application. This is a system that takes input from sensors; performs some application-specific processing on the data from the sensors; and sends appropriate values to output devices. 

- Keep detailed and up-to-date documentation.

- Performing rigorous testing at each increment.

-----

### Stage 1: ANALYSIS and INITIAL DESIGN

Identify the functionality requirements for your application, as well as the types of inputs and outputs necessary.

The initial design documentation should contain at least:
- An overall description of the system to be built, outlining the functionality that will be achieved.
- Initial plans as to what sensors / devices will be used.
- An initial analysis of timing requirements.
- A block diagram or flow chart showing the key functional behaviour.

----

### Stage 2: DETAILED DESIGN
Design the system. The design documentation should include:

- A mapping of the real-world environment and context information onto the inputs of the control system (i.e. what are the inputs and where do they come from), and also a mapping of the outputs of the control system onto the real-world showing the effects of the control behaviour (i.e. what are the outputs and what do they control).

- Identification of IO sensors / devices.
- Details of I/O ports used and their configurations.
- Interrupt system configuration (including priorities if multiple interrupts are used).

- Details of timing requirements and how these will be met. This should include event periodicity, scheduling, prioritisation and WCET limits and also include details of the ways in which programmable timers are used.

- Timing diagrams conforming to the UML 2.0 standard.
- Flowcharts or other flow / structure representations of the proposed software structure showing key code modules, interrupt handlers, libraries used etc.
- State machine diagrams.

----

### Stage 3: DEVELOPMENT

#### Basic Solution specification
In terms of functionality, a basic solution is expected to achieve approximately the following level of functionality:
- A Real-Time timing-related aspect (and must use at least 1 programmable timer).
- An asynchronous aspect (must use at least 1 interrupt channel and must have an appropriate interrupt handler).
- Use either: at least 1 type of input and 2 types of output, or at least 2 types of input and 1 type of output.

#### Extended Solutions
Extended solutions might incorporate some techniques and features such as:
- A RT constraint: e.g. some form of deadline or periodically scheduled activity.
- Use of multiple timers.
- Multiple interrupts, with appropriate consideration of prioritisation.
- Use the ADC or the Analogue Comparator.
- Use of more-complex devices, e.g. LCD, Keypad, and any custom peripherals.
- Structuring the program code into libraries to improve the organisation and re-use of code.
- In-built diagnostics, e.g. diagnostic patterns on a bank of LEDs to show the program state, or diagnostic data output via USART to display/log on computer.

-----

### Stage 4: TESTING
Develop a testing methodology which includes a set of specific tests and justification of these tests. The tests should encompass (where appropriate):
- Timing aspects, including Worst-Case Execution Time (WCET) analysis for all timing critical sections.
- Benchmarking of timing/behaviour/performance.
- Function / robustness / reliability of each feature supported.
- Correctness of operation, with specific reference to the Real-Time constraints of the specific application.
- Usability aspects.

Apply the tests objectively, under an extended range of conditions, and report the results in a clear, unbiased fashion.

-----

### Stage 5: EVALUATION
You should critically evaluate your system in terms of:

- How well it meets your original design specification, and with hindsight, the appropriateness of the design (i.e. its suitability-for-purpose).
- Improvements or enhancements that could be carried out if there were further time available.
- You should also discuss what you have learned from completing this task.



----

### Deliverables:
Each student should submit a single report which includes all of the deliverables:

- Clear design documentation (Relates to stage 1 and mainly stage 2). This must include an essay explaining how the program works (this should normally be about one full page of 12-point font, but if you have a more-complex program requiring more explanation then please expand the essay part appropriately). In particular you should discuss how the various functions of the microcontroller have been utilised. (Relates to stage 3).

- Test methodology and detailed analysis of results (Relates to stage 4).

- Program listings. The code should be well-structured and self-documentation features such as meaningful variable names and comments should be used. Use comments to clearly differentiate code that you have created or modified from any sample code that you have used, and provide clear comments to state the sources of any sample code that is used (Relates to stage 3).

- Critical evaluation and conclusion (approximately 500 words) (Relates to stage 5).
- The total documentation for the report should not exceed 15 pages (not including program listings).
- Diagrams must conform to the relevant UML standards where appropriate.
