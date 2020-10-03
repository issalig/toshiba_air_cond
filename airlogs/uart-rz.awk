BEGIN{n=0;}
{
        /* Grab six bytes */
        while(n != 6) {
                /* The first byte is always A8, so wait for that */
                while(n == 0 && $2 != "A8")
                        getline;
                /* Only take data from the UART */
                if(/^uart-1:/) {
                        /* Save it to an array */
                        d[n] = $2;
                        n++;
                        /* Next line */
                        getline;
                }
        };

        /* Print time and all the data */
        printf("%s ", strftime("%Y-%m-%d %H:%M:%S"));
        for(i = 0; i < n; i++) {
                printf(" %s", d[i]);
        }
        printf("\n");
        fflush();
        /* Reset for next cycle */
        n = 0;
}
