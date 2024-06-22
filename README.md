<h1>Hot Potato Game - C Socket Implementation</h1>
    <h2>Introduction</h2>
    <p>This project implements a client-server application using Berkeley sockets in C to simulate the Hot Potato game. The game involves guessing a numerical answer within a certain number of attempts. Each attempt informs the player whether the correct answer is higher or lower than the guess, or if the guess is correct.</p>
    
  <h2>Development Environment</h2>
    <p>The development environment consists of:</p>
    <ul>
        <li>Debian GNU/Linux 9 (stretch) workstation (nogal.usal.es)</li>
        <li>Berkeley Sockets</li>
        <li>C programming language</li>
    </ul>
    
  <h2>Protocol Specifications</h2>
    <p>The Hot Potato service (PC) uses a specific protocol for communication between clients and servers. This involves sending and receiving predefined messages formatted with specific end-of-line characters and length constraints.</p>
    
  <h3>Message Format</h3>
    <p>Messages in the PC protocol are lines of characters terminated with CR-LF (carriage return '\r' and line feed '\n'). The maximum length of a message line is 516 bytes, including the CR-LF characters.</p>
    
  <h3>Client to Server Messages</h3>
    <ul>
        <li><strong>HOLA:</strong> This is the initial message sent by the client. Format: <code>HOLA[CR-LF]</code></li>
        <li><strong>RESPUESTA:</strong> Sends a numerical guess to the server. Format: <code>RESPUESTA &lt;number&gt;[CR-LF]</code>, where <code>&lt;number&gt;</code> is the guessed number.</li>
        <li><strong>+</strong>: Requests another question after a correct guess or when attempts are exhausted. Format: <code>+[CR-LF]</code></li>
        <li><strong>ADIOS:</strong> Ends the game. Format: <code>ADIOS[CR-LF]</code></li>
    </ul>
    
   <h2>Program Details</h2>
    <h3>Server Implementation</h3>
    <p>The server handles incoming client connections, processes their guesses, and provides feedback on whether the correct answer is higher, lower, or correct. It also manages the game state and ensures that the protocol rules are followed.</p>
    
  <h3>Client Implementation</h3>
    <p>The client connects to the server, initiates the game with the HOLA message, sends numerical guesses using the RESPUESTA message, and can request new questions or end the game using the + and ADIOS messages respectively.</p>
    
  <h2>How to Compile and Run</h2>
    <p>To compile the program, use the following command:</p>
    <pre><code>gcc -o hot_potato_server server.c</code></pre>
    <pre><code>gcc -o hot_potato_client client.c</code></pre>
    <p>To run the server:</p>
    <pre><code>./hot_potato_server &lt;port_number&gt;</code></pre>
    <p>To run the client:</p>
    <pre><code>./hot_potato_client &lt;server_ip&gt; &lt;port_number&gt;</code></pre>
    
  <h2>Conclusion</h2>
    <p>This project demonstrates a simple yet engaging use of sockets in C to implement a networked game. The Hot Potato game serves as a practical example of client-server communication, message formatting, and game state management.</p>
